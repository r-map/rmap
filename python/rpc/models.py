from django.db import models
from django.contrib.auth.models import User
from rmap.stations.models import UserProfile, StationMetadata
from django.core.exceptions import ValidationError
from django.utils.translation import ugettext_lazy as _
from django.conf import settings
from jsonfield import JSONField
from django.utils import timezone
import json

class Rpc(models.Model):
    dbid = models.AutoField(primary_key=True)
    stationmetadata = models.ForeignKey(StationMetadata,on_delete=models.CASCADE)
    id = models.IntegerField(editable=False,default=0)
    active = models.BooleanField(_("Active"),default=True,null=False,blank=False,help_text=_("Active ticket"))
    date = models.DateTimeField(_('Date'), default=timezone.now)
    datecmd = models.DateTimeField(_('Date command'), default=None, null=True,blank=True)
    method = models.CharField(_('method'), max_length=40, null=False)
    params = JSONField(_('params'), max_length=120, null=True,blank=True)
    dateres = models.DateTimeField(_('Date reposnse'), default=None, null=True,blank=True)
    result = JSONField(_('result'), max_length=120, null=True,blank=True)
    error =  JSONField(_('error'),  max_length=120, null=True,blank=True)

    def params_json(self):
        return json.dumps(self.params)
                  
    def status(self):
        if (self.datecmd is not None):
            if (self.dateres is not None):
                return "completed"
            else:
                return "running"
        else:
            if (self.active):
                return "submitted"

        if (self.active):
            if (self.datecmd is not None) or (self.dateres is not None):
                return "sequence error"

    def __str__(self):
        return str(self.method) + " " + str(self.params) + " " + str(self.active) + " " + str(self.status())

            
    class Meta:
        ordering = ('-date','-id',)
        verbose_name = _('RPC')
        verbose_name_plural = _('RPCSs')

    def save(self, force_insert=False, force_update=False):
        # Only modify number if creating for the first time (is default 0)
        if self.id == 0:
            # Grab the highest current index (if it exists)
            try:
                last = Rpc.objects.filter(stationmetadata__exact=self.stationmetadata).order_by('-id')[0]
                self.id = last.id + 1
            except IndexError:
                self.id = 1
        # Call the "real" save() method
        super(Rpc, self).save(force_insert, force_update)
