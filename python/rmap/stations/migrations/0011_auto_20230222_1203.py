# Generated by Django 2.2.17 on 2023-02-22 12:03

from django.db import migrations, models


class Migration(migrations.Migration):

    dependencies = [
        ('stations', '0010_auto_20230116_1416'),
    ]

    operations = [
        migrations.AlterField(
            model_name='sensor',
            name='address',
            field=models.PositiveIntegerField(default=None, help_text='I2C address (decimal)', null=True),
        ),
        migrations.AlterField(
            model_name='sensor',
            name='i2cbus',
            field=models.PositiveIntegerField(blank=True, default=None, help_text='I2C bus number (for raspberry only)', null=True),
        ),
        migrations.AlterField(
            model_name='sensor',
            name='node',
            field=models.PositiveIntegerField(blank=True, default=None, help_text='RF24Network node ddress', null=True),
        ),
    ]
