# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('stations', '0003_fixture'),
    ]

    operations = [
        migrations.AlterField(
            model_name='stationmetadata',
            name='slug',
            field=models.SlugField(help_text='Auto-generated from name.'),
        ),
    ]
