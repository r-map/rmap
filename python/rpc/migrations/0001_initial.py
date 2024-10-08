# Generated by Django 2.2.17 on 2023-08-01 00:29

from django.db import migrations, models
import django.db.models.deletion
import django.utils.timezone
import jsonfield.fields


class Migration(migrations.Migration):

    initial = True

    dependencies = [
        ('stations', '0036_auto_20230710_0909'),
    ]

    operations = [
        migrations.CreateModel(
            name='Rpc',
            fields=[
                ('dbid', models.AutoField(primary_key=True, serialize=False)),
                ('id', models.IntegerField(default=0, editable=False)),
                ('active', models.BooleanField(default=True, help_text='Active ticket', verbose_name='Active')),
                ('date', models.DateTimeField(default=django.utils.timezone.now, verbose_name='Date')),
                ('datecmd', models.DateTimeField(blank=True, default=None, null=True, verbose_name='Date command')),
                ('method', models.CharField(max_length=40, verbose_name='method')),
                ('params', jsonfield.fields.JSONField(blank=True, max_length=120, null=True, verbose_name='params')),
                ('dateres', models.DateTimeField(blank=True, default=None, null=True, verbose_name='Date reposnse')),
                ('result', jsonfield.fields.JSONField(blank=True, max_length=120, null=True, verbose_name='result')),
                ('error', jsonfield.fields.JSONField(blank=True, max_length=120, null=True, verbose_name='error')),
                ('stationmetadata', models.ForeignKey(on_delete=django.db.models.deletion.CASCADE, to='stations.StationMetadata')),
            ],
            options={
                'verbose_name': 'RPC',
                'verbose_name_plural': 'RPCSs',
                'ordering': ('-date', '-id'),
            },
        ),
    ]
