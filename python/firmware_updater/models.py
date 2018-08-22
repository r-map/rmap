

from django.db import models
from django.utils.translation import ugettext_lazy

from  django import VERSION as djversion

if ((djversion[0] == 1 and djversion[1] >= 3) or 
    djversion[0] > 1):

    from django.db import models
    from django.db.models import signals

    class DeletingFileField(models.FileField):
        """
        FileField subclass that deletes the refernced file when the model object
        itself is deleted.
        
        WARNING: Be careful using this class - it can cause data loss! This class
        makes at attempt to see if the file's referenced elsewhere, but it can get
        it wrong in any number of cases.
        """
        def contribute_to_class(self, cls, name):
            super(DeletingFileField, self).contribute_to_class(cls, name)
            signals.post_delete.connect(self.delete_file, sender=cls)
        
        def delete_file(self, instance, sender, **kwargs):
            file = getattr(instance, self.attname)
            # If no other object of this type references the file,
            # and it's not the default value for future objects,
            # delete it from the backend.
            
            if file and file.name != self.default and \
                    not sender._default_manager.filter(**{self.name: file.name}):
                file.delete(save=False)
            elif file:
                # Otherwise, just close the file, so it doesn't tie up resources.
                file.close()

else:
    DeletingFileField=models.FileField


#class NameManager(models.Manager):
#    def get_by_natural_key(self, name):
#        #print "NameManager: ",name
#        return self.get(name=name)

class Name(models.Model):
    """Firmware metadata."""

#    objects = NameManager()

    name = models.CharField(max_length=30,unique=True,default="",blank=False,help_text=ugettext_lazy("Firmware name"))
    description = models.CharField(max_length=300,default="",blank=True,help_text=ugettext_lazy("Firmware description"))

#    def natural_key(self):
#        #print "natural key sensor type"
#        #print self,self.type, self.board.natural_key()
#        return (self.name,)

    class Meta:
        ordering = ['name']
        verbose_name = 'Firmware name' 
        verbose_name_plural = 'Firmware names' 

    def __unicode__(self):
        return '%s' % (self.name)


class Firmware(models.Model):
	
    #firmware = models.CharField(ugettext_lazy("Firmware name"),max_length=80,unique=False)
    firmware = models.ForeignKey(Name,on_delete=models.CASCADE)
    file = DeletingFileField(ugettext_lazy('File'),upload_to='firmware',max_length=255,\
                    help_text=ugettext_lazy("The firmware file to upload"))
    date = models.DateTimeField(ugettext_lazy('Build date'),\
                    help_text=ugettext_lazy("When the firmware was done"))
    active = models.BooleanField(ugettext_lazy("Active"),default=True,\
                    help_text=ugettext_lazy("Activate the firmware for upgrade"))

    unique_together = (('firmware', 'date'),)
        

    class Meta:
        ordering = ['date']
        verbose_name = 'Firmware' 
        verbose_name_plural = 'Firmware' 

    def __unicode__(self):
        return '%s' % (self.firmware)
