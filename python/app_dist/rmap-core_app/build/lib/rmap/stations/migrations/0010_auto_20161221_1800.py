# -*- coding: utf-8 -*-
# Generated by Django 1.9.11 on 2016-12-21 18:00


from django.db import migrations, models
import django.db.models.deletion


class Migration(migrations.Migration):

    dependencies = [
        ('stations', '0009_auto_20161014_1320'),
    ]

    operations = [
        migrations.CreateModel(
            name='Bcode',
            fields=[
                ('id', models.AutoField(auto_created=True, primary_key=True, serialize=False, verbose_name='ID')),
                ('bcode', models.CharField(default=b'B00000', help_text='Bcode as defined in dballe btable', max_length=6)),
                ('description', models.CharField(default=b'Undefined', help_text='Descriptive text', max_length=100)),
                ('unit', models.CharField(default=b'Undefined', help_text='units of measure', max_length=20)),
            ],
            options={
                'ordering': ['bcode'],
                'verbose_name': 'Variable Bcode',
                'verbose_name_plural': 'Variable Bcode',
            },
        ),
        migrations.CreateModel(
            name='SensorType',
            fields=[
                ('id', models.AutoField(auto_created=True, primary_key=True, serialize=False, verbose_name='ID')),
                ('active', models.BooleanField(default=False, help_text='Activate this sensor to take measurements', verbose_name='Active')),
                ('name', models.CharField(default=b'my sensor type', help_text='Descriptive text', max_length=100)),
                ('type', models.CharField(choices=[(b'TMP', b'I2C TMP temperature sensor'), (b'ADT', b'I2C ADT temperature sensor'), (b'BMP', b'I2C BMP085/BMP180 pressure sensor'), (b'HIH', b'I2C HIH6100 series humidity sensor'), (b'DW1', b'I2C Davis/Inspeed/Windsonic wind direction and intensity adapter'), (b'TBR', b'I2C Tipping bucket rain gauge adapter'), (b'RF24', b'RF24 Network jsonrpc'), (b'STH', b'I2C TH module, one shot mode'), (b'ITH', b'I2C TH module, report mode, istantaneous values'), (b'NTH', b'I2C TH module, report mode, minimum values'), (b'MTH', b'I2C TH module, report mode, mean values'), (b'XTH', b'I2C TH module, report mode, maximum values'), (b'SSD', b'I2C SDS011 module, one shot mode')], default=b'TMP', help_text='Type of sensor', max_length=4)),
                ('bcodes', models.ManyToManyField(help_text='Bcode variable definition', to='stations.Bcode')),
            ],
            options={
                'ordering': ['type'],
                'verbose_name': 'Sensor Type',
                'verbose_name_plural': 'Sensors Type',
            },
        ),
        migrations.AlterField(
            model_name='sensor',
            name='type',
            field=models.ForeignKey(help_text='Type of sensor', on_delete=django.db.models.deletion.CASCADE, to='stations.SensorType'),
        ),
    ]