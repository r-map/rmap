# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations
from django.core import serializers
import os

fixture_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '../fixtures'))
#fixture_filename = 'initial_data.json'

def load_fixture(apps, schema_editor):

    #print ""
    #print "Insert password for  user 'rmap' (administrator superuser)"
    #call_command("createsuperuser",username="rmap",email="rmap@rmap.cc") 
    from django.core.management import call_command
    call_command("createsuperuser",username="rmap",email="rmap@rmap.cc",interactive=False) 

    from django.contrib.auth.models import User
    u = User.objects.get(username__exact='rmap')
    u.set_password('rmap')
    u.save()

    for fixture_filename in ("site_rmap.cc.json",):
        if fixture_filename[-5:] == ".json":
            
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

    #MyModel = apps.get_model("stations", "StationMetadata")
    #MyModel.objects.all().delete()
    #MyModel = apps.get_model("stations", "Board")
    #MyModel.objects.all().delete()
    #MyModel = apps.get_model("stations", "Sensor")
    #MyModel.objects.all().delete()

    #    MyModel = apps.get_model("stations", "UserProfile")
    #    MyModel.objects.get(user=apps.get_model("auth", "User").objects.get(username="rmap")).delete()
    #    apps.get_model("auth", "User").objects.get(username="rmap").delete()

    pass


class Migration(migrations.Migration):

    dependencies = [
        ('stations', '0002_initial'),
    ]

    operations = [
        migrations.RunPython(load_fixture, reverse_code=unload_fixture),
    ]
