from djgeojson.fields import PointField
from django.db import models

class GeorefencedImage(models.Model):

    geom = PointField()
    description = models.TextField()
    image = models.ImageField()

    @property
    def popupContent(self):

      return '<a href="/{}"><img src="/{}" style="width:128px;height:128px;" ></a><p>{}</p>'.format(
          self.image.url,
          self.image.url,
          self.description)

