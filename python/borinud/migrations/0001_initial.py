# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
    ]

    operations = [
        migrations.CreateModel(
            name='Source',
            fields=[
                ('id', models.AutoField(verbose_name='ID', serialize=False, auto_created=True, primary_key=True)),
                ('connection_type', models.IntegerField(choices=[(1, b'dballe'), (2, b'arkimet bufr'), (3, b'arkimet vm2')])),
                ('connection', models.CharField(max_length=512)),
            ],
            options={
            },
            bases=(models.Model,),
        ),
        migrations.CreateModel(
            name='Summary',
            fields=[
                ('slug', models.CharField(max_length=512, serialize=False, primary_key=True)),
                ('ident', models.CharField(max_length=9, null=True, blank=True)),
                ('network', models.CharField(max_length=16)),
                ('lon', models.IntegerField(null=True)),
                ('lat', models.IntegerField(null=True)),
                ('tr', models.IntegerField(null=True)),
                ('p1', models.IntegerField(null=True)),
                ('p2', models.IntegerField(null=True)),
                ('lt1', models.IntegerField(null=True)),
                ('lv1', models.IntegerField(null=True)),
                ('lt2', models.IntegerField(null=True)),
                ('lv2', models.IntegerField(null=True)),
                ('var', models.CharField(max_length=6, null=True, blank=True)),
                ('lonmin', models.IntegerField(null=True)),
                ('latmin', models.IntegerField(null=True)),
                ('lonmax', models.IntegerField(null=True)),
                ('latmax', models.IntegerField(null=True)),
                ('datemin', models.DateTimeField(null=True, blank=True)),
                ('datemax', models.DateTimeField(null=True, blank=True)),
                ('timestamp', models.DateTimeField(auto_now=True)),
                ('source', models.ForeignKey(to='borinud.Source')),
            ],
            options={
            },
            bases=(models.Model,),
        ),
    ]
