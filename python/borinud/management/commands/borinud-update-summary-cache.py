from django.core.management.base import BaseCommand, CommandError


class Command(BaseCommand):
    help = "Update summary cache"

    def handle(self, *args, **options):
        from borinud.utils.source import get_db
        db = get_db()
        if hasattr(db, "write_cached_summary"):
            db.write_cached_summary()
