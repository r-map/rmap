from __future__ import unicode_literals
from django.db import models
from django.utils.translation import ugettext_lazy

class NetworkMetadata(models.Model):
    name = models.SlugField(primary_key=True,max_length=80,help_text=ugettext_lazy("Network name"))
    description = models.CharField(max_length=200,help_text=ugettext_lazy("Network description"))
    provider = models.CharField(max_length=200,blank=True,help_text=ugettext_lazy("Data provider"))
    provider_url = models.URLField(blank=True,help_text=ugettext_lazy("Main site of the data provider"))
    licenze = models.CharField(max_length=200,blank=True,help_text=ugettext_lazy("Data licenze"))
    licenze_url = models.URLField(blank=True,help_text=ugettext_lazy("Data licenze url"))

