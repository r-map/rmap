from imagekit.models import ImageSpecField
from imagekit.models import ProcessedImageField
from imagekit.processors import ResizeToFill, Transpose, SmartResize, ResizeToFit
from  django import VERSION as djversion
from django.db import models
from django.contrib.auth.models import User
from rmap.stations.models import UserProfile, StationMetadata
from django.core.exceptions import ValidationError
import os
from django.utils.translation import ugettext_lazy as _
from django.conf import settings
import datetime,mimetypes


if ((djversion[0] == 1 and djversion[1] >= 3) or 
    djversion[0] > 1):

    from django.db.models import signals

    class DeletingImageField(ProcessedImageField):
        """
        ProcessedImageField subclass that deletes the refernced file when the model object
        itself is deleted.
        
        WARNING: Be careful using this class - it can cause data loss! This class
        makes at attempt to see if the file's referenced elsewhere, but it can get
        it wrong in any number of cases.
        """
        def contribute_to_class(self, cls, name):
            super(DeletingImageField, self).contribute_to_class(cls, name)
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


    class DeletingAttachmentField(models.FileField):
        """
        ProcessedAttachmentField subclass that deletes the refernced file when the model object
        itself is deleted.
        
        WARNING: Be careful using this class - it can cause data loss! This class
        makes at attempt to see if the file's referenced elsewhere, but it can get
        it wrong in any number of cases.
        """
        def contribute_to_class(self, cls, name):
            super(DeletingAttachmentField, self).contribute_to_class(cls, name)
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
    DeletingImageField=models.ImageField
    DeletingFileField=models.FileField




PRIORITY_CHOICES = (
    (1, _('Critical')),
    (2, _('High')),
    (3, _('Normal')),
    (4, _('Low')),
    (5, _('Very Low')),
)

class Ticket(models.Model):
    stationmetadata = models.ForeignKey(StationMetadata,on_delete=models.CASCADE)
    ticket = models.AutoField(editable=False, primary_key=True)
    active = models.BooleanField(_("Active"),default=True,null=False,blank=False,help_text=_("Active ticket"))
    date = models.DateTimeField(
        _('Data apertura'),
        default=datetime.datetime.now)
    priority = models.IntegerField(
        _('Priority'),
        choices=PRIORITY_CHOICES,
        default=3)
    abstract = models.CharField(
        _('Abstract of the problem'),
        max_length=40,
        null=True)
    description = models.TextField(
        _('Description of the problem'),
        max_length=500)

    assigned_to = models.ManyToManyField(User, related_name='assigned_to', blank=True, verbose_name=_('Assigned to'),
    )
    subscribed_by = models.ManyToManyField(User, related_name='subscribed_by', blank=True, verbose_name=_('Subscribed by'),
    )    

    
    def active_txt(self):
        if self.active:
            return ugettext('Open')
        else:
            return ugettext('Closed')

    def __unicode__(self):
        return self.nome + " " + self.date.__str__() + " " + self.active_txt()

ACTIONTICKET_CATEGORY_CHOICES = (
    ('onsite', 'Action on site'),
    ('remote', 'Action fron remote'),
    ('server', 'Action on server'),
    ('others', 'Others'),
)

class TicketAction(models.Model):
    active = models.BooleanField(_("Active"),default=True,null=False,blank=False,help_text=_("Activate this action"))
    ticket = models.ForeignKey(Ticket,on_delete=models.CASCADE)
    date = models.DateTimeField(
        _('Date and time'),
        default=datetime.datetime.now)
    category = models.CharField(max_length=50, blank=False,choices=ACTIONTICKET_CATEGORY_CHOICES)    
    description = models.TextField(
        _('Description of the action'), blank=True, max_length=500)

    def __unicode__(self):
        return self.date.isoformat()


PHOTOTICKET_CATEGORY_CHOICES = (
    ('station','Station description'),
    ('sensors','Sensors description'),
    ('others', 'Others'),
)

class TicketImage(models.Model):

    active = models.BooleanField(_("Active"),default=True,null=False,blank=False,help_text=_("Activate this station image"))
    description = models.TextField(_('Description of the image'), blank=True, max_length=500)
    ticket = models.ForeignKey(Ticket,on_delete=models.CASCADE)
    date=models.DateTimeField(auto_now=True, auto_now_add=False)
    category = models.CharField(max_length=50, blank=False,choices=PHOTOTICKET_CATEGORY_CHOICES)
    image = DeletingImageField(upload_to='ticketimage',processors=[Transpose(),ResizeToFit(1024, 1024)],
                                          format='jpeg',
                                          options={'quality': 70})

    image_thumbnail = ImageSpecField(
        source='image',
        processors = [Transpose(),SmartResize(128, 128)],
        format = 'JPEG',
        options = {'quality': 60}
    )


def validate_file_extension(value):
    ext = os.path.splitext(value.name)[1]  # [0] returns path+filename
    # TODO: we might improve this with more thorough checks of file types
    # rather than just the extensions.

    # check if VALID_EXTENSIONS is defined in settings.py
    # if not use defaults

    if hasattr(settings, 'VALID_EXTENSIONS'):
        valid_extensions = settings.VALID_EXTENSIONS
    else:
        valid_extensions = ['.txt', '.asc', '.htm', '.html',
                            '.pdf', '.doc', '.docx', '.odt', '.jpg', '.png', '.eml']

    if not ext.lower() in valid_extensions:
        # TODO: one more check in case it is a file with no extension; we
        # should always allow that?
        if not (ext.lower() == '' or ext.lower() == '.'):
            raise ValidationError(
                _('Unsupported file extension: ') + ext.lower()
            )
    
class TicketAttachment(models.Model):

    ticket = models.ForeignKey(Ticket,on_delete=models.CASCADE)
    active = models.BooleanField(_("Active"),default=True,null=False,blank=False,help_text=_("Activate this station image"))
    date=models.DateTimeField(auto_now=True, auto_now_add=False)
    
    file = models.FileField(
        _('File'),
        upload_to="tickets_attachments",
        max_length=1000,
        validators=[validate_file_extension]
    )

    filename = models.CharField(
        _('Filename'),
        blank=True,
        max_length=1000,
    )

    mime_type = models.CharField(
        _('MIME Type'),
        blank=True,
        max_length=255,
    )

    size = models.IntegerField(
        _('Size'),
        blank=True,
        help_text=_('Size of this file in bytes'),
    )

    def __str__(self):
        return '%s' % self.filename

    def save(self, *args, **kwargs):

        if not self.size:
            self.size = self.get_size()

        if not self.filename:
            self.filename = self.get_filename()

        if not self.mime_type:
            self.mime_type = \
                mimetypes.guess_type(self.filename, strict=False)[0] or \
                'application/octet-stream'

        return super(TicketAttachment, self).save(*args, **kwargs)

    def get_filename(self):
        return str(self.file)

    def get_size(self):
        return self.file.file.size

    class Meta:
        ordering = ('filename',)
        verbose_name = _('Attachment')
        verbose_name_plural = _('Attachments')

