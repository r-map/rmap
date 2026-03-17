from django.db import migrations, models
from django.core import serializers
import os

fixture_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '../fixtures'))

def load_fixture(apps, schema_editor):
    
    def load_fixture_from_file(fixture_file):
        fixture = open(fixture_file, 'rb')
        objects = serializers.deserialize('json', fixture, ignorenonexistent=True)
        for obj in objects:
            obj.save()
        fixture.close()


    fixture_filename = "station_airquality-mobile.json"
    fixture_file = os.path.join(fixture_dir, fixture_filename)
    print("load fixture from file: ",fixture_file)
    load_fixture_from_file(fixture_file)

    

def unload_fixture(apps, schema_editor):
    "Brutally deleting all entries for this model..."

    #MyModel = apps.get_model("stations", "StationMetadata")
    #MyModel.objects.all().delete()
    #MyModel = apps.get_model("stations", "Board")
    #MyModel.objects.all().delete()
    #MyModel = apps.get_model("stations", "Sensor")
    #MyModel.objects.all().delete()
    pass

class Migration(migrations.Migration):

    dependencies = [
        ('stations', '0042_auto_20260312_0617'),
    ]

    operations = [
        migrations.RunPython(
            code=load_fixture,
            reverse_code=unload_fixture,
        ),                
    ]
