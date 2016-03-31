from imagekit.models import ImageSpecField
from imagekit.models import ProcessedImageField
from imagekit.processors import ResizeToFill, Transpose, SmartResize, ResizeToFit
from djgeojson.fields import PointField
from django.db import models
from django.contrib.auth.models import User
from django.utils.translation import ugettext_lazy
from  django import VERSION as djversion

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

else:
    DeletingImageField=ProcessedImageField


CATEGORY_CHOICES = (
    ('meteo','Meteo phenomena'),
    ('others', 'Others'),
)


class GeorefencedImage(models.Model):

    active = models.BooleanField(ugettext_lazy("Active"),default=True,null=False,blank=False,help_text=ugettext_lazy("Activate this geoimage"))
    geom = PointField()
    comment = models.TextField()
    #image = DeletingImageField()
    ident = models.ForeignKey(User)
    date=models.DateTimeField(auto_now=False, auto_now_add=False)
    category = models.CharField(max_length=50, blank=False,choices=CATEGORY_CHOICES)


    image = DeletingImageField(processors=[Transpose(),ResizeToFit(1024, 1024)],
                                          format='jpeg',
                                          options={'quality': 70})

    image_thumbnail = ImageSpecField(
        source='image',
        processors = [Transpose(),SmartResize(128, 128)],
        format = 'JPEG',
        options = {'quality': 60}
    )

    @property
    def popupContent(self):
        return \
            u'\
            <p>\
            <a href="#" onClick="window.open(\'/geoimage/{}/{}\',\'geoimage\', \'width=800, height=620\').focus(); return false;" >\
            <img src="/{}" style="float:right;">\
            </a>\
            {}\
            </p>\
            <p><a href="/geoimage/{}">{}</a> {}</p>'.format(
                self.ident,
                self.id,
                self.image_thumbnail.url,
                self.comment,
                self.ident,
                self.ident,
                self.date
            )

