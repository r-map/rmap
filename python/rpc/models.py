from django.db import models
from django.contrib.auth.models import User
from rmap.stations.models import UserProfile, StationMetadata
from django.core.exceptions import ValidationError
from django.utils.translation import ugettext_lazy as _
from django.conf import settings
from jsonfield import JSONField
from django.utils import timezone


class Rpc(models.Model):
    stationmetadata = models.ForeignKey(StationMetadata,on_delete=models.CASCADE)
    id = models.AutoField(editable=False, primary_key=True)
    active = models.BooleanField(_("Active"),default=True,null=False,blank=False,help_text=_("Active ticket"))
    date = models.DateTimeField(_('Date'), default=timezone.now)
    datecmd = models.DateTimeField(_('Date command'), default=None, null=True,blank=True)
    method = models.CharField(_('method'), max_length=40, null=False)
    params = JSONField(_('params'), max_length=120, null=True,blank=True)
    dateres = models.DateTimeField(_('Date reposnse'), default=None, null=True,blank=True)
    result = JSONField(_('result'), max_length=120, null=True,blank=True)
    error =  JSONField(_('error'),  max_length=120, null=True,blank=True)
    
    def __unicode__(self):
        return self.method + " " + self.date.__str__() + " " + self.active

    def status(self):
        if (self.active):
            if (self.datecmd is not None):
                if (self.dateres is not None):
                    return "completed"
                else:
                    return "running"
            else:
                return "submitted"
        else:
            if (self.datecmd is not None) and (self.datecmd is not None):
                return "completed"
            else:
                return "sequence error"
    
    class Meta:
        ordering = ('-date','-id',)
        verbose_name = _('RPC')
        verbose_name_plural = _('RPCSs')

