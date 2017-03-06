# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations
from django.core import serializers
import os

fixture_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '../fixtures'))
#fixture_filename = 'initial_data.json'

def load_fixture(apps, schema_editor):

    fixture_file=fixture_dir+"/sensor_type_01.json"
    fixture = open(fixture_file, 'rb')
    objects = serializers.deserialize('json', fixture, ignorenonexistent=True)
    for obj in objects:
        obj.save()
        fixture.close()

def unload_fixture(apps, schema_editor):
    "Brutally deleting all entries for this model..."

    MyModel = apps.get_model("stations", "StationMetadata")
    MyModel.objects.all().delete()
    MyModel = apps.get_model("stations", "Board")
    MyModel.objects.all().delete()
    MyModel = apps.get_model("stations", "Sensor")
    MyModel.objects.all().delete()

class Migration(migrations.Migration):

    dependencies = [
        ('stations', '0012_auto_20170120_0909'),
    ]

    operations = [
        migrations.RunPython(load_fixture, reverse_code=unload_fixture),
    ]
