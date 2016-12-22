# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations
from django.core import serializers
import os

fixture_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '../fixtures'))
#fixture_filename = 'initial_data.json'

def load_fixture(apps, schema_editor):

    fixture_file=fixture_dir+"/sensor_type.json"
    fixture = open(fixture_file, 'rb')
    objects = serializers.deserialize('json', fixture, ignorenonexistent=True)
    for obj in objects:
        obj.save()
        fixture.close()


    for fixture_filename in os.listdir(fixture_dir):
        if fixture_filename[:3] == "sta" and fixture_filename[-5:] == ".json":
            
            fixture_file = os.path.join(fixture_dir, fixture_filename)
            print "load fixture from file: ",fixture_file

            fixture = open(fixture_file, 'rb')
            objects = serializers.deserialize('json', fixture, ignorenonexistent=True)
            for obj in objects:
                obj.save()
            fixture.close()

#def load_fixture(apps, schema_editor):
#    call_command('loaddata', 'initial_data', app_label='stations') 

def unload_fixture(apps, schema_editor):
    "Brutally deleting all entries for this model..."

    MyModel = apps.get_model("stations", "StationMetadata")
    MyModel.objects.all().delete()
    MyModel = apps.get_model("stations", "Board")
    MyModel.objects.all().delete()
    MyModel = apps.get_model("stations", "Sensor")
    MyModel.objects.all().delete()

#    MyModel = apps.get_model("stations", "UserProfile")
#    MyModel.objects.get(user=apps.get_model("auth", "User").objects.get(username="rmap")).delete()
#    apps.get_model("auth", "User").objects.get(username="rmap").delete()

class Migration(migrations.Migration):

    dependencies = [
        ('stations', '0010_auto_20161221_1800'),
    ]

    operations = [
        migrations.RunPython(load_fixture, reverse_code=unload_fixture),
    ]
