%define name django-geojson
%define version 2.9.0
%define unmangled_version 2.9.0
%define unmangled_version 2.9.0
%define release 1

Summary: Serve vectorial map layers with Django
Name: %{name}
Version: %{version}
Release: %{release}
Source0: %{name}-%{unmangled_version}.tar.gz
License: LPGL, see LICENSE file.
Group: Development/Libraries
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-buildroot
Prefix: %{_prefix}
BuildArch: noarch
Vendor: Mathieu Leplatre <mathieu.leplatre@makina-corpus.com>
Url: https://github.com/makinacorpus/django-geojson

%description
*django-geojson* is a set of tools to manipulate GeoJSON with Django :

* (De)Serializer for (Geo)Django objects, querysets and lists
* Base views to serve GeoJSON map layers from models
* GeoJSON model and form fields to avoid spatial database backends
  (compatible with *django-leaflet* for map widgets)


.. image:: https://pypip.in/v/django-geojson/badge.png
        :target: https://pypi.python.org/pypi/django-geojson

.. image:: https://pypip.in/d/django-geojson/badge.png
        :target: https://pypi.python.org/pypi/django-geojson

.. image:: https://travis-ci.org/makinacorpus/django-geojson.png?branch=master
    :target: https://travis-ci.org/makinacorpus/django-geojson

.. image:: https://coveralls.io/repos/makinacorpus/django-geojson/badge.png?branch=master
    :target: https://coveralls.io/r/makinacorpus/django-geojson


=======
INSTALL
=======

::

    pip install django-geojson


This package has **optional** `extra dependencies <http://pythonhosted.org/setuptools/setuptools.html#declaring-extras-optional-features-with-their-own-dependencies>`_.


If you need GeoJSON fields with map widgets :

::

    pip install "django-geojson [field]"


=====
USAGE
=====

Add ``djgeojson`` to your applications :

::

    # settings.py

    INSTALLED_APPS += (
        'djgeojson',
    )

*(not required for views)*


GeoJSON layer view
==================

Very useful for web mapping :

::

    # urls.py
    from djgeojson.views import GeoJSONLayerView
    ...
    url(r'^data.geojson$', GeoJSONLayerView.as_view(model=MushroomSpot), name='data'),


Consume the vector layer as usual, for example, with Leaflet loaded with jQuery Ajax:

::

    # Leaflet JS
    var layer = L.geoJson();
    map.addLayer(layer);
    $.getJSON("{% url 'data' %}", function (data) {
        layer.addData(data);
    });


Inherit generic views **only** if you need a reusable set of options :

::

    # views.py
    from djgeojson.views import GeoJSONLayerView

    class MapLayer(GeoJSONLayerView):
        # Options
        precision = 4   # float
        simplify = 0.5  # generalization


    # urls.py
    from .views import MapLayer, MeetingLayer
    ...
    url(r'^mushrooms.geojson$', MapLayer.as_view(model=MushroomSpot, properties=('name',)), name='mushrooms')

Most common use-cases of reusable options are: low-fi precision, common list of fields between several views, etc.

Options are :

* **properties** : ``list`` of properties names, or ``dict`` for mapping field names and properties
* **simplify** : generalization of geometries (See ``simplify()``)
* **precision** : number of digit after comma
* **geometry_field** : name of geometry field (*default*: ``geom``)
* **srid** : projection (*default*: 4326, for WGS84)
* **bbox** : Allows you to set your own bounding box on feature collection level
* **bbox_auto** : True/False (default false). Will automatically generate a bounding box on a per feature level.
* **use_natural_keys** : serialize natural keys instead of primary keys (*default*: ``False``)


Tiled GeoJSON layer view
========================

Vectorial tiles can help display a great number of objects on the map,
with `reasonnable performance <https://groups.google.com/forum/?fromgroups#!searchin/leaflet-js/GeoJSON$20performance$3F$20River$20vector$20tile$20map./leaflet-js/_WJquNpdmH0/qQsasZpCTPUJ>`_.

::

    # urls.py
    from djgeojson.views import TiledGeoJSONLayerView
    ...

    url(r'^data/(?P<z>\d+)/(?P<x>\d+)/(?P<y>\d+).geojson$',
        TiledGeoJSONLayerView.as_view(model=MushroomSpot), name='data'),


Consume the vector tiles using `Leaflet TileLayer GeoJSON <https://github.com/glenrobertson/leaflet-tilelayer-geojson/>`_, `Polymaps <http://polymaps.org/>`_, `OpenLayers 3 <http://twpayne.github.io/ol3/remote-vector/examples/tile-vector.html>`_ or `d3.js <http://d3js.org>`_ for example.

Options are :
 
* **trim_to_boundary** : if ``True`` geometries are trimmed to the tile boundary
* **simplifications** : a dict of simplification values by zoom level



GeoJSON template filter
=======================

Mainly useful to dump features in HTML output and bypass AJAX call :

::

    // Leaflet JS
    L.geoJson({{ object_list|geojsonfeature|safe}}).addTo(map);


Will work either for a model, a geometry field or a queryset.

::

    {% load geojson_tags %}
    
    var feature = {{ object|geojsonfeature|safe }};
    
    var geom = {{ object.geom|geojsonfeature|safe }};

    var collection = {{ object_list|geojsonfeature|safe }};


Properties and custom geometry field name can be provided.

::

    {{ object|geojsonfeature:"name,age" }}
    {{ object|geojsonfeature:"name,age:the_geom" }}
    {{ object|geojsonfeature:":geofield" }}


Model and forms fields
======================

GeoJSON fields are based on Brad Jasper's `JSONField <https://pypi.python.org/pypi/jsonfield>`_.
See `INSTALL`_ to install extra dependencies.

They are useful to avoid usual GIS stacks (GEOS, GDAL, PostGIS...)
for very simple use-cases (no spatial operation yet).

::

    from djgeojson.fields import PointField

    class Address(models.Model):
        geom = PointField()

    address = Address()
    address.geom = {'type': 'Point', 'coordinates': [0, 0]}
    address.save()


Form widgets are rendered with Leaflet maps automatically if
`django-leaflet <https://github.com/makinacorpus/django-leaflet>`_
is available.

All geometry types are supported and respectively validated :
`GeometryField`, `PointField`, `MultiPointField`, `LineStringField`,
`MultiLineStringField`, `PolygonField`, `MultiPolygonField`,
`GeometryCollectionField`.


Low-level serializer
====================

::

    from djgeojson.serializers import Serializer as GeoJSONSerializer

    GeoJSONSerializer().serialize(Restaurants.objects.all(), use_natural_keys=True, with_modelname=False)



Low-level deserializer
======================

::

    from djgeojson.serializers import Serializer as GeoJSONSerializer

    GeoJSONSerializer().deserialize('geojson', my_geojson)

You can optionally specify the model name directly in the parameters:

::

    GeoJSONSerializer().deserialize('geojson', my_geojson, model_name=my_model_name)




Dump GIS models, or fixtures
============================

Register the serializer in your project :

::

    # settings.py

    SERIALIZATION_MODULES = {
        'geojson' : 'djgeojson.serializers'
    }

Command-line ``dumpdata`` can export files, viewable in GIS software like QGis :

::

    python manage.py dumpdata --format=geojson yourapp.Model > export.geojson

Works with ``loaddata`` as well, which can now import GeoJSON files.



=======
AUTHORS
=======

    * Mathieu Leplatre <mathieu.leplatre@makina-corpus.com>
    * Glen Robertson author of django-geojson-tiles at: https://github.com/glenrobertson/django-geojson-tiles/
    * @jeffkistler's author of geojson serializer at: https://gist.github.com/967274
    * Ben Welsh and Lukasz Dziedzia for `quick test script <http://datadesk.latimes.com/posts/2012/06/test-your-django-app-with-travisci/>`_
    * Florent Lebreton http://github.com/fle
    * Julien Le Sech http://www.idreammicro.com
    * Kevin Cooper @kevcooper
    * Achille Ash @AchilleAsh

Version 1.X:

    * Daniel Sokolowski, serializer snippet
    * ozzmo, python 2.6 compatibility

|makinacom|_

.. |makinacom| image:: http://depot.makina-corpus.org/public/logo.gif
.. _makinacom:  http://www.makina-corpus.com

=======
LICENSE
=======

    * Lesser GNU Public License


=========
CHANGELOG
=========

2.9.0 (2016-02-08)
==================

** New features **

- handle natural keys in views (#74, thanks Achille Ash!)

** Bug fixes **

- Add Django 1.9 compatibility (#69, thanks Julien Le Sech!)
- Fix imports in view.py to work without GEOS (#62, thanks Kevin Cooper!)


2.8.1 (2015-06-17)
==================

** Bug fixes**

- Fixed detection of GEOS (thanks Kevin Cooper!)

2.8.0 (2015-04-17)
==================

** New features **

- Support GeoJSON specification for named crs (thanks Alvin Lindstam)

** Bug fixes **

- Add python 3.2 compatibility (thanks Nikolay Korotkiy, fixes #55)
- Fix tests on Django >= 1.7 (thanks Manel Clos)


2.7.0 (2015-02-21)
==================

** New features **

- Add a with_modelname option to serializer

** Bug fixes **

- change 'fields' to 'properties' in code example
- Adds a warning for "Module version, as defined in PEP-0396


2.6.0 (2014-07-21)
==================

** New features **

- Django GeoJSON fields without libgeos installed (thanks Florent Lebreton)
- Properties can be a tuple (fixes #34)


2.5.0 (2014-06-03)
==================

** New features **

- Add vector tiles view
- Improved `geojsonfeature` template tag (fixes #15, #16)
- Add various GeoJSON fields, for each geometry type

** Bug fixes **

- Fix (de)serializers not being usable from command-line (fixes #28)
- Fix import attempt for *django-leaflet* (fixes #27), by Seyi Ogunyemi
- Fix missed SRID after copying a geometry in ``_handle_geom``, by Biel Frontera

** Internal changes **

- Specify django-leaflet minimal version for GeoJSON model field
- Got rid of shapely for deserialization


2.4.0 (2014-03-22)
==================

- Add GeoJSON fields


2.3.0 (2014-02-08)
==================

- Python 3 support (thanks @amarandon)
- Add bbox at feature level (thanks @7wonders)


2.2.0 (2013-12-18)
==================

- Deserialization: add ability to specify model name as option (thanks @Vross)
- Deserialization: look-up ``ìd`` value in properties whe missing at feature level: (thanks @Vross)


2.1.1 (2013-08-21)
==================

- Set default SRID to 4326 in generic GeoJSON views.


2.1.0 (2013-08-19)
==================

- Serialize reversed relations using natural keys (fixes #8)
- Support empty geometries (None or NULL in Db)
- Fix serializing property in upper class

2.0.1 (2013-07-10)
==================

- Fix usage of simplify.
- Expose ``force2d`` option in view
- Allow to have ``pk`` or ``id`` in properties if explicitly listed

2.0.0 (2013-07-09)
==================

- Complete rewrite using @jeffkistler and @glenrobertson code
- CRS is added to GeoJSON ouput by default
- Ability to build ``pk`` dynamically by passing a lambda
- Ability to provide a ``bbox``
- Ability to use Django natural keys
- Support of *ValuesQuerySet*
- Support of *ForeignKey* and *ManyToMany*
- Added ``force2d`` option

** Backwards incompatible changes **

- Geom field is not guessed automatically : Use ``geometry_field`` option, default is ``'geom'``.
- no more ``pk`` in properties : use feature ``id``, or list ``pk`` in properties explicitly.
- ``fields`` list in ``GeoJSONLayer`` was renamed ``properties``.

1.2.0 (2013-05-22)
==================

- Shapely is now optional (used for deserialization only)
- Add Django to requirements
- Skip a step in GeoJSON conversion of geometries (fixes #6)


1.1.0 (2013-03-06)
==================

- Django 1.5 support
- Fix template tag geojsonfeature on empty geometries

1.0.0 (2012-08-03)
==================

- Initial working version.


%prep
%setup -n %{name}-%{unmangled_version} -n %{name}-%{unmangled_version}

%build
python setup.py build

%install
python setup.py install --single-version-externally-managed -O1 --root=$RPM_BUILD_ROOT --record=INSTALLED_FILES

%clean
rm -rf $RPM_BUILD_ROOT

%files -f INSTALLED_FILES
%defattr(-,root,root)
