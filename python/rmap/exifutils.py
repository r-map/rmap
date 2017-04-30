#!/usr/bin/env python
# This Python file uses the following encoding: utf-8
"""
Utility to adjust the EXIF
"""

import sys
import piexif
from datetime import datetime
import io

class Rational:
    """A simple fraction class. Python 2.6 could use the inbuilt Fraction class."""

    def __init__(self, num, den):
        """Create a number fraction num/den."""
        self.num = num
        self.den = den

    def __repr__(self):
        """Return a string representation of the fraction."""
        return "%s / %s" % (self.num, self.den)

    def as_tuple(self):
        """Return the fraction a numerator, denominator tuple."""
        return (self.num, self.den)



def get_geo(exif_dict):
    """Return a tuple of (latitude, longitude)."""
    def convert(x):
        deg=Rational(x[0][0],x[0][1])
        min=Rational(x[1][0],x[1][1])
        sec=Rational(x[2][0],x[2][1])

        return (float(deg.num) / deg.den) +  \
            (1/60.0 * float(min.num) / min.den) + \
            (1/3600.0 * float(sec.num) / sec.den)

    lat = convert(exif_dict["GPS"][piexif.GPSIFD.GPSLatitude])
    lng = convert(exif_dict["GPS"][piexif.GPSIFD.GPSLongitude])
    if exif_dict["GPS"][piexif.GPSIFD.GPSLatitudeRef] == "S":
        lat = -lat
    if exif_dict["GPS"][piexif.GPSIFD.GPSLongitudeRef] == "W":
        lng = -lng

    return lat, lng

SEC_DEN = 50000000

def parse(val):
    sign = 1
    if val < 0:
        val = -val
        sign = -1

    deg = int(val)
    other = (val - deg) * 60
    minutes = int(other)
    secs = (other - minutes) * 60
    secs = long(secs * SEC_DEN)
    return (sign, deg, minutes, secs)

#_parse = staticmethod(_parse)

def set_geo(exif_dict, lat, lng):
    """Set the GeoLocation to a given lat and lng"""

    sign, deg, min, sec = parse(lat)
    ref = "N"
    if sign < 0:
        ref = "S"

    exif_dict["GPS"][piexif.GPSIFD.GPSLatitudeRef] = ref

    exif_dict["GPS"][piexif.GPSIFD.GPSLatitude] = [Rational(deg, 1).as_tuple(),
                       Rational(min, 1).as_tuple(),
                       Rational(sec, SEC_DEN).as_tuple()]

    sign, deg, min, sec = parse(lng)
    ref = "E"
    if sign < 0:
        ref = "W"
    exif_dict["GPS"][piexif.GPSIFD.GPSLongitudeRef] = ref
    exif_dict["GPS"][piexif.GPSIFD.GPSLongitude] = [Rational(deg, 1).as_tuple(),
                        Rational(min, 1).as_tuple(),
                        Rational(sec, SEC_DEN).as_tuple()]

def dumpimage(data):

        exif_dict = piexif.load(data)

        for ifd in ("0th", "Exif", "GPS", "1st"):
            for tag in exif_dict[ifd]:
                print tag
                print(piexif.TAGS[ifd][tag]["name"], exif_dict[ifd][tag])
        
        try:
            lat,lon=get_geo(exif_dict)
            print "lat lon",lat,lon
        except:
            print "error getting lat lon metadata"

def setgeoimage(data,lat,lon,imagedescription="",usercomment="",dt=None):

        exif_dict = piexif.load(data)

        #for ifd in ("0th", "Exif", "GPS", "1st"):
        #    for tag in exif_dict[ifd]:
        #        print tag
        #        print(piexif.TAGS[ifd][tag]["name"], exif_dict[ifd][tag])
        #
        #lat,lon=get_geo(exif_dict)
        #print "lat lon",lat,lon

        if dt is None:
             exif_dict["0th"][piexif.ImageIFD.DateTime]=datetime.utcnow().strftime("%Y:%m:%d %H:%M:%S")
        else:
             exif_dict["0th"][piexif.ImageIFD.DateTime]=dt.strftime("%Y:%m:%d %H:%M:%S")
        set_geo(exif_dict, lat, lon)
        exif_dict["Exif"][piexif.ExifIFD.UserComment]=chr(0x55)+chr(0x4E)+chr(0x49)+chr(0x43)+chr(0x4F)+chr(0x44)+chr(0x45)+chr(0x00)+usercomment
        exif_dict["0th"][piexif.ImageIFD.ImageDescription]=imagedescription
        exif_bytes = piexif.dump(exif_dict)

        new_data=io.BytesIO(data)
        piexif.insert(exif_bytes, data, new_data)

        return new_data.getvalue()

def getgeoimage(data):

        exif_dict = piexif.load(data)
        try:
            lat,lon=get_geo(exif_dict)
        except:
            lat=None
            lon=None
        try:
            imagedescription=exif_dict["0th"][piexif.ImageIFD.ImageDescription]
        except:
            imagedescription=None
        try:
            usercomment=exif_dict["Exif"][piexif.ExifIFD.UserComment][8:]
        except:
            usercomment=None

        try:
            dt=datetime.strptime(exif_dict["0th"][piexif.ImageIFD.DateTime], '%Y:%m:%d %H:%M:%S')
        except:
            dt=None
        return lat,lon,imagedescription,usercomment,dt


def photo_manage(filename):
    """
    resize (and rotate) image 
    """
    #size_mini = 200, 200
    size_maxi = 1024, 1024
    try:
        try:
            from PIL import Image as PILImage
        except:
            import Image as PILImage
    except:
        print "To use this program, you need to install Python Imaging Library PILLOW"
        sys.exit(1)

    im = PILImage.open(filename)

    try:
        exif_dict = piexif.load(im.info["exif"])
    except:
        exif_dict={"0th":{}}

    # We create the thumbnail
    im.thumbnail(size_maxi, PILImage.ANTIALIAS)

    ## We rotate regarding to the EXIF orientation information
    if piexif.ImageIFD.Orientation in exif_dict["0th"]:
        orientation = exif_dict["0th"][piexif.ImageIFD.Orientation]

        print "ECCO ORIENTATION:",orientation

        if orientation == 1:
            # Nothing
            mirror = im.copy()
        elif orientation == 2:
            # Vertical Mirror
            mirror = im.transpose(PILImage.FLIP_LEFT_RIGHT)
        elif orientation == 3:
            # Rotation 180
            mirror = im.transpose(PILImage.ROTATE_180)
        elif orientation == 4:
            # Horizontal Mirror
            mirror = im.transpose(PILImage.FLIP_TOP_BOTTOM)
        elif orientation == 5:
            # Horizontal Mirror + Rotation 270 CCW
            mirror = im.transpose(PILImage.FLIP_TOP_BOTTOM).transpose(PILImage.ROTATE_270)
        elif orientation == 6:
            # Rotation 90
            mirror = im.transpose(PILImage.ROTATE_270)
        elif orientation == 7:
            # Horizontal Mirror + Rotation 90
            mirror = im.transpose(PILImage.FLIP_TOP_BOTTOM).transpose(PILImage.ROTATE_90)
        elif orientation == 8:
            # Rotation 90
            mirror = im.transpose(PILImage.ROTATE_90)
        # No more Orientation information
        exif_dict["0th"][piexif.ImageIFD.Orientation] = 1
    else:
        # No EXIF information, the user has to do it
        mirror = im.copy()

    exif_bytes = piexif.dump(exif_dict)
    mirror.save(filename, "JPEG", quality=70,exif=exif_bytes)

    #img_grand = pyexiv2.Image(grand)
    #img_grand.readMetadata()
    #image.copyMetadataTo(img_grand)
    #img_grand.writeMetadata()
    #print grand
    
    #mirror.thumbnail(size_mini, PILImage.ANTIALIAS)
    #mirror.save(mini, "JPEG", quality=85)
    #img_mini = pyexiv2.Image(mini)
    #img_mini.readMetadata()
    #image.copyMetadataTo(img_mini)
    #img_mini.writeMetadata()
    #print mini


if __name__ == "__main__":


    from optparse import OptionParser

    def parse_args():
        p = OptionParser(usage='%prog file.jpg...',
           description='adjusts timestamps in EXIF metadata')
        p.add_option("-d", "--dump",
                  action="store_true", dest="dump", default=False,
                  help="dump exif metadata")

        options, args = p.parse_args()
        if len(args) < 1:
            p.error('not enough arguments')
        return  options,args

    def main():
        options,files = parse_args()

        for fname in files:

            photo_manage(fname)

            with open(fname) as file:
                data = file.read()

                if options.dump:
                     print "Input metadata:"
                     dumpimage(data)

                new_data=setgeoimage(data,lat=44.,lon=11.,imagedescription="pat1",usercomment="prova")

                if options.dump:
                     print "Output metadata:"
                     dumpimage(new_data)

                with open(fname+"new","w") as file:
                    file.write(new_data)

        return 0

    sys.exit(main())
