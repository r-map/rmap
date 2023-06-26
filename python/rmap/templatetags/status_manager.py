from django import template
from django.utils.safestring import mark_safe

register = template.Library()

def statusb(value):  # Only one argument.
    """Converts a status bit to somethings better to display"""

    if (value is None):
        result = " "
    elif (value):
        result = "<i class=\"bi bi-bug\"></i>"
    else:
        result = "<i class=\"bi bi-check-square\"></i>" 

    return mark_safe(result)

register.filter("statusb", statusb)

