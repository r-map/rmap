# Generated by Django 2.2.17 on 2023-07-10 09:09

from django.db import migrations, models
import rmap.stations.models


class Migration(migrations.Migration):

    dependencies = [
        ('stations', '0035_fixture'),
    ]

    operations = [
        migrations.AlterField(
            model_name='board',
            name='name',
            field=models.CharField(help_text='board name', max_length=255),
        ),
        migrations.AlterField(
            model_name='stationmetadata',
            name='category',
            field=models.CharField(choices=[('good', 'Beautifull & Good'), ('bad', 'Bad & Wrong'), ('test', 'Test & Bugs'), ('template', 'Used to generate new stations'), ('unknown', 'Unknown & Missing')], default='unknown', help_text='Category of the station', max_length=50),
        ),
        migrations.AlterField(
            model_name='transportmqtt',
            name='mqttpassword',
            field=models.CharField(blank=True, default=rmap.stations.models.TransportMqtt.genpassword, help_text='MQTT password', max_length=50, null=True),
        ),
    ]
