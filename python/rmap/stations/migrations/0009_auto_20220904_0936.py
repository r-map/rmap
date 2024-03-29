# Generated by Django 2.2.17 on 2022-09-04 09:36

from django.db import migrations, models


class Migration(migrations.Migration):

    dependencies = [
        ('stations', '0008_auto_20220712_0640'),
    ]

    operations = [
        migrations.AddField(
            model_name='stationmaintstatus',
            name='firmwaremajor',
            field=models.PositiveIntegerField(blank=True, default=None, help_text='firmware major version', null=True),
        ),
        migrations.AddField(
            model_name='stationmaintstatus',
            name='firmwareminor',
            field=models.PositiveIntegerField(blank=True, default=None, help_text='firmware minor version', null=True),
        ),
    ]
