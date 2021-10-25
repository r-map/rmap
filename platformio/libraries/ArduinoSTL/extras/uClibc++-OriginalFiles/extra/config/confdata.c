/*
 * Copyright (C) 2002 Roman Zippel <zippel@linux-m68k.org>
 * Released under the terms of the GNU GPL v2.0.
 */

#include <sys/stat.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define LKC_DIRECT_LINK
#include "lkc.h"

const char conf_def_filename[] = ".config";

const char conf_defname[] = "extra/Configs/Config.default";

const char *conf_confnames[] = {
	".config",
	conf_defname,
	NULL,
};

static char *conf_expand_value(const char *in)
{
	struct symbol *sym;
	const char *src;
	static char res_value[SYMBOL_MAXLENGTH];
	char *dst, name[SYMBOL_MAXLENGTH];

	res_value[0] = 0;
	dst = name;
	while ((src = strchr(in, '$'))) {
		strncat(res_value, in, src - in);
		src++;
		dst = name;
		while (isalnum(*src) || *src == '_')
			*dst++ = *src++;
		*dst = 0;
		sym = sym_lookup(name, 0);
		sym_calc_value(sym);
		strcat(res_value, sym_get_string_value(sym));
		in = src;
	}
	strcat(res_value, in);

	return res_value;
}

char *conf_get_default_confname(void)
{
	struct stat buf;
	static char fullname[4096+1];
	char *env, *name;

	name = conf_expand_value(conf_defname);
	env = getenv(SRCTREE);
	if (env) {
		sprintf(fullname, "%s/%s", env, name);
		if (!stat(fullname, &buf))
			return fullname;
	}
	return name;
}

int conf_read(const char *name)
{
	FILE *in = NULL;
	char line[1024];
	char *p, *p2;
	int lineno = 0;
	struct symbol *sym;
	struct property *prop;
	struct expr *e;
	int i;

	if (name) {
		in = zconf_fopen(name);
	} else {
		const char **names = conf_confnames;
		while ((name = *names++)) {
			name = conf_expand_value(name);
			in = zconf_fopen(name);
			if (in) {
				printf("#\n"
				       "# using defaults found in %s\n"
				       "#\n", name);
				break;
			}
		}
	}

	if (!in)
		return 1;

	for_all_symbols(i, sym) {
		sym->flags |= SYMBOL_NEW | SYMBOL_CHANGED;
		sym->flags &= ~SYMBOL_VALID;
		switch (sym->type) {
		case S_INT:
		case S_HEX:
		case S_STRING:
			if (sym->user.val)
				free(sym->user.val);
		default:
			sym->user.val = NULL;
			sym->user.tri = no;
		}
	}

	while (fgets(line, sizeof(line), in)) {
		lineno++;
		sym = NULL;
		switch (line[0]) {
		case '#':
			if (line[1]!=' ')
				continue;
			p = strchr(line + 2, ' ');
			if (!p)
				continue;
			*p++ = 0;
			if (strncmp(p, "is not set", 10))
				continue;
			sym = sym_find(line + 2);
			if (!sym) {
				fprintf(stderr, "%s:%d: trying to assign nonexistent symbol %s\n", name, lineno, line + 2);
				break;
			}
			switch (sym->type) {
			case S_BOOLEAN:
			case S_TRISTATE:
				sym->user.tri = no;
				sym->flags &= ~SYMBOL_NEW;
				break;
			default:
				;
			}
			break;

		case 'A' ... 'Z':
			p = strchr(line, '=');
			if (!p)
				continue;
			*p++ = 0;
			p2 = strchr(p, '\n');
			if (p2)
				*p2 = 0;
			sym = sym_find(line);
			if (!sym) {
				fprintf(stderr, "%s:%d: trying to assign nonexistent symbol %s\n", name, lineno, line);
				break;
			}
			switch (sym->type) {
			case S_TRISTATE:
				if (p[0] == 'm') {
					sym->user.tri = mod;
					sym->flags &= ~SYMBOL_NEW;
					break;
				}
			case S_BOOLEAN:
				if (p[0] == 'y') {
					sym->user.tri = yes;
					sym->flags &= ~SYMBOL_NEW;
					break;
				}
				if (p[0] == 'n') {
					sym->user.tri = no;
					sym->flags &= ~SYMBOL_NEW;
					break;
				}
				break;
			case S_STRING:
				if (*p++ != '"')
					break;
				for (p2 = p; (p2 = strpbrk(p2, "\"\\")); p2++) {
					if (*p2 == '"') {
						*p2 = 0;
						break;
					}
					memmove(p2, p2 + 1, strlen(p2));
				}
				if (!p2) {
					fprintf(stderr, "%s:%d: invalid string found\n", name, lineno);
					exit(1);
				}
			case S_INT:
			case S_HEX:
				if (sym_string_valid(sym, p)) {
					sym->user.val = strdup(p);
					sym->flags &= ~SYMBOL_NEW;
				} else {
					fprintf(stderr, "%s:%d: symbol value '%s' invalid for %s\n", name, lineno, p, sym->name);
					exit(1);
				}
				break;
			default:
				;
			}
			break;
		case '\n':
			break;
		default:
			continue;
		}
		if (sym && sym_is_choice_value(sym)) {
			struct symbol *cs = prop_get_symbol(sym_get_choice_prop(sym));
			switch (sym->user.tri) {
			case no:
				break;
			case mod:
				if (cs->user.tri == yes)
					/* warn? */;
				break;
			case yes:
				if (cs->user.tri != no)
					/* warn? */;
				cs->user.val = sym;
				break;
			}
			cs->user.tri = E_OR(cs->user.tri, sym->user.tri);
			cs->flags &= ~SYMBOL_NEW;
		}
	}
	fclose(in);

	for_all_symbols(i, sym) {
		sym_calc_value(sym);
		if (sym_has_value(sym) && !sym_is_choice_value(sym)) {
			if (sym->visible == no)
				sym->flags |= SYMBOL_NEW;
			switch (sym->type) {
			case S_STRING:
			case S_INT:
			case S_HEX:
				if (!sym_string_within_range(sym, sym->user.val))
					sym->flags |= SYMBOL_NEW;
			default:
				break;
			}
		}
		if (!sym_is_choice(sym))
			continue;
		prop = sym_get_choice_prop(sym);
		for (e = prop->expr; e; e = e->left.expr)
			if (e->right.sym->visible != no)
				sym->flags |= e->right.sym->flags & SYMBOL_NEW;
	}

	sym_change_count = 1;

	return 0;
}

int conf_write(const char *name)
{
	FILE *out, *out_h;
	struct symbol *sym;
	struct menu *menu;
	const char *basename;
	char dirname[128], tmpname[128], newname[128];
	int type, l;
	const char *str;

	dirname[0] = 0;
	if (name && name[0]) {
		char *slash = strrchr(name, '/');
		if (slash) {
			int size = slash - name + 1;
			memcpy(dirname, name, size);
			dirname[size] = 0;
			if (slash[1])
				basename = slash + 1;
			else
				basename = conf_def_filename;
		} else
			basename = name;
	} else
		basename = conf_def_filename;

	sprintf(newname, "%s.tmpconfig.%d", dirname, getpid());
	out = fopen(newname, "w");
	if (!out)
		return 1;
	out_h = NULL;
	if (!name) {
		out_h = fopen(".tmpconfig.h", "w");
		if (!out_h)
			return 1;
	}
	fprintf(out, "#\n"
		     "# Automatically generated make config: don't edit\n"
		     "#\n");
	if (out_h)
		fprintf(out_h, "/*\n"
			     " * Automatically generated C config: don't edit\n"
			     " */\n"
			     "/*\n"
			     " * Version Number\n"
			     " */\n"
			     "#define __UCLIBCXX_MAJOR__ %s\n"
			     "#define __UCLIBCXX_MINOR__ %s\n"
			     "#define __UCLIBCXX_SUBLEVEL__ %s\n",
			     getenv("MAJOR_VERSION"),
			     getenv("MINOR_VERSION"),
			     getenv("SUBLEVEL"));

	if (!sym_change_count)
		sym_clear_all_valid();

	menu = rootmenu.list;
	while (menu) {
		sym = menu->sym;
		if (!sym) {
			if (!menu_is_visible(menu))
				goto next;
			str = menu_get_prompt(menu);
			fprintf(out, "\n"
				     "#\n"
				     "# %s\n"
				     "#\n", str);
			if (out_h)
				fprintf(out_h, "\n"
					       "/*\n"
					       " * %s\n"
					       " */\n", str);
		} else if (!(sym->flags & SYMBOL_CHOICE)) {
			sym_calc_value(sym);
			if (!(sym->flags & SYMBOL_WRITE))
				goto next;
			sym->flags &= ~SYMBOL_WRITE;
			type = sym->type;
			if (type == S_TRISTATE) {
				sym_calc_value(modules_sym);
				if (modules_sym->curr.tri == no)
					type = S_BOOLEAN;
			}
			switch (type) {
			case S_BOOLEAN:
			case S_TRISTATE:
				switch (sym_get_tristate_value(sym)) {
				case no:
					fprintf(out, "# %s is not set\n", sym->name);
					if (out_h)
						fprintf(out_h, "#undef __%s__\n", sym->name);
					break;
				case mod:
					fprintf(out, "%s=m\n", sym->name);
					if (out_h)
						fprintf(out_h, "#define %s_MODULE 1\n", sym->name);
					break;
				case yes:
					fprintf(out, "%s=y\n", sym->name);
					if (out_h)
						fprintf(out_h, "#define __%s__ 1\n", sym->name);
					break;
				}
				break;
			case S_STRING:
				// fix me
				str = sym_get_string_value(sym);
				fprintf(out, "%s=\"", sym->name);
				if (out_h)
					fprintf(out_h, "#define __%s__ \"", sym->name);
				do {
					l = strcspn(str, "\"\\");
					if (l) {
						fwrite(str, l, 1, out);
						if (out_h)
							fwrite(str, l, 1, out_h);
					}
					str += l;
					while (*str == '\\' || *str == '"') {
						fprintf(out, "\\%c", *str);
						if (out_h)
							fprintf(out_h, "\\%c", *str);
						str++;
					}
				} while (*str);
				fputs("\"\n", out);
				if (out_h)
					fputs("\"\n", out_h);
				break;
			case S_HEX:
				str = sym_get_string_value(sym);
				if (str[0] != '0' || (str[1] != 'x' && str[1] != 'X')) {
					fprintf(out, "%s=%s\n", sym->name, str);
					if (out_h)
						fprintf(out_h, "#define __%s__ 0x%s\n", sym->name, str);
					break;
				}
			case S_INT:
				str = sym_get_string_value(sym);
				fprintf(out, "%s=%s\n", sym->name, str);
				if (out_h)
					fprintf(out_h, "#define __%s__ %s\n", sym->name, str);
				break;
			}
		}

	next:
		if (menu->list) {
			menu = menu->list;
			continue;
		}
		if (menu->next)
			menu = menu->next;
		else while ((menu = menu->parent)) {
			if (menu->next) {
				menu = menu->next;
				break;
			}
		}
	}
	fclose(out);
	if (out_h) {
		fclose(out_h);
		rename(".tmpconfig.h", "include/system_configuration.h");
		file_write_dep(NULL);
	}
	if (!name || basename != conf_def_filename) {
		if (!name)
			name = conf_def_filename;
		sprintf(tmpname, "%s.old", name);
		rename(name, tmpname);
	}
	sprintf(tmpname, "%s%s", dirname, basename);
	if (rename(newname, tmpname))
		return 1;

	sym_change_count = 0;

	return 0;
}
