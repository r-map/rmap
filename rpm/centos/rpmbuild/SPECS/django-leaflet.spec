%define name django-leaflet
%define version 0.18.0
%define unmangled_version 0.18.0
%define unmangled_version 0.18.0
%define release 1

Summary: Use Leaflet in your django projects
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
Url: https://github.com/makinacorpus/django-leaflet

%description
*django-leaflet* allows you to use `Leaflet <http://leafletjs.com>`_
in your `Django <https://www.djangoproject.com>`_ projects.

It embeds Leaflet in version *0.7.5*.

.. image:: https://pypip.in/v/django-leaflet/badge.png
        :target: https://pypi.python.org/pypi/django-leaflet

.. image:: https://pypip.in/d/django-leaflet/badge.png
        :target: https://pypi.python.org/pypi/django-leaflet

.. image:: https://travis-ci.org/makinacorpus/django-leaflet.png
    :target: https://travis-ci.org/makinacorpus/django-leaflet

.. image:: https://coveralls.io/repos/makinacorpus/django-leaflet/badge.png
    :target: https://coveralls.io/r/makinacorpus/django-leaflet


Main purposes of having a python package for the Leaflet Javascript library :

* Install and enjoy ;
* Do not embed Leaflet assets in every Django project ;
* Enjoy geometry edition with Leaflet form widget ;
* Control apparence and settings of maps from Django settings (e.g. at deployment) ;
* Reuse Leaflet map initialization code (e.g. local projections) ;

:note:

    *django-leaflet* is compatible with `django-geojson <https://github.com/makinacorpus/django-geojson.git>`_ fields, which
    allow handling geographic data without spatial database.

=======
INSTALL
=======

Last stable version:

::

    pip install django-leaflet


Last development version (master branch):

::

    pip install -e git+https://github.com/makinacorpus/django-leaflet.git#egg=django-leaflet


=====
USAGE
=====

* Add ``leaflet`` to your ``INSTALLED_APPS``

* Make sure ``django.contrib.staticfiles`` is also in your ``INSTALLED_APPS``; Django >= 1.3 `includes this contrib app by default <https://docs.djangoproject.com/en/1.3/intro/tutorial01/#database-setup>`_

* Add the HTML header::

    {% load leaflet_tags %}

    <head>
        ...
        {% leaflet_js %}
        {% leaflet_css %}
    </head>

* Add the map in your page, providing a name::

    ...
    <body>
        ...
        {% leaflet_map "yourmap" %}
        ...
    </body>

* Your map shows up!


Use Leaflet API
---------------

You can use the *Leaflet* API as usual. There are two ways to
grab a reference on the just initialized map and options.


**Using Javascript callback function**

The easy way :

::

    <script type="text/javascript">
        function map_init_basic (map, options) {
            ...
            L.marker([50.5, 30.5]).addTo(map);
            ...
        }
    </script>

    {% leaflet_map "yourmap" callback="window.map_init_basic" %}


**Using events**

If you don't want to expose global callbacks :

::

    <script type="text/javascript">
        window.addEventListener("map:init", function (e) {
            var detail = e.detail;
            ...
            L.marker([50.5, 30.5]).addTo(detail.map);
            ...
        }, false);
    </script>

Event object has two properties : ``map`` and ``options`` (initialization).

For Internet Explorer support, we fallback on jQuery if available ::

    $(window).on('map:init', function (e) {
        var detail = e.originalEvent ?
                     e.originalEvent.detail : e.detail;
        ...
        L.marker([50.5, 30.5]).addTo(detail.map);
        ...
    });

If you want to support archaic browsers **and** still avoid jQuery,
*django-leaflet* comes with a minimalist polyfill for events.
Add it in ``<head>`` this way ::

    <!--[if IE 8]><script src="{% static "leaflet/eventlister.ie8.js" %}"></script><!--<![endif]-->
    <!--[if lt IE 8]><script src="{% static "leaflet/eventlister.ie6-7.js" %}"></script><!--<![endif]-->


Customize map size
------------------

CSS is your friend:

::

    <style>

        .leaflet-container {  /* all maps */
            width:  600px;
            height: 400px;
        }

        #specialbigmap {
            height: 800px;
        }

    </style>



Configuration
-------------

In order to configure *django-leaflet*, just add a new section in your
settings::

    LEAFLET_CONFIG = {
        # conf here
    }

And add some of the following entries.


Spatial extent
~~~~~~~~~~~~~~

You can configure a global spatial extent for your maps, that will
automatically center your maps, restrict panning and add reset view and scale
controls. (*See advanced usage to tweak that.*)::

    'SPATIAL_EXTENT': (5.0, 44.0, 7.5, 46)


Initial map center and zoom level
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In addition to limiting your maps with ``SPATIAL_EXTENT``, you can also specify
initial map center, default, min and max zoom level::

    'DEFAULT_CENTER': (6.0, 45.0),
    'DEFAULT_ZOOM': 16,
    'MIN_ZOOM': 3,
    'MAX_ZOOM': 18,

The tuple/list must contain (lat,lng) coords.


Default tiles layer
~~~~~~~~~~~~~~~~~~~

To globally add a tiles layer to your maps::

    'TILES': 'http://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png'

This setting can also be a list of tuples ``(name, url, options)``.
The python dict ``options`` accepts all the Leaflet tileLayers options.

If it contains several layers, a layer switcher will then be added automatically.

::

    'TILES': [('Satellite', 'http://server/a/...', {'attribution': '&copy; Big eye', 'maxZoom': 16}),
              ('Streets', 'http://server/b/...', {'attribution': '&copy; Contributors'})]


Overlay layers
~~~~~~~~~~~~~~

To globally add an overlay layer, use the same syntax as tiles::

    'OVERLAYS': [('Cadastral', 'http://server/a/{z}/{x}/{y}.png', {'attribution': '&copy; IGN'})]

Currently, overlay layers from settings are limited to tiles. For vectorial overlays, you
will have to add them via JavaScript (see events).


Attribution prefix
~~~~~~~~~~~~~~~~~~

To globally add an attribution prefix on maps (most likely an empty string) ::

    'ATTRIBUTION_PREFIX': 'Powered by django-leaflet'

Default is ``None``, which leaves the value to `Leaflet's default <http://leafletjs.com/reference.html#control-attribution>`_.


Scale control
~~~~~~~~~~~~~

Scale control may be set to show 'metric' (m/km), or 'imperial' (mi/ft) scale
lines, or 'both'.  Default is 'metric'.

Enable metric and imperial scale control::

    'SCALE': 'both'

Disable scale control::

    'SCALE': None


Minimap control
~~~~~~~~~~~~~~~

Shows a small map in the corner which shows the same as the main map with a
set zoom offset::

    'MINIMAP': True

By default it shows the tiles of the first layer in the list.

(`More info... <https://github.com/Norkart/Leaflet-MiniMap>`_)

Reset view button
~~~~~~~~~~~~~~~~~
By default, a button appears below the zoom controls and, when clicked, shows the entire map.
To remove this button, set::

    'RESET_VIEW': False


Global initialization functions and ``window.maps``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Since 0.7.0, the ``leaflet_map`` template tag no longer registers initialization functions in global scope,
and no longer adds map objects into ``window.maps`` array by default. To restore these features, use::

    'NO_GLOBALS' = False

Force Leaflet image path
~~~~~~~~~~~~~~~~~~~~~~~~~~~
If you are using staticfiles compression libraries such as django_compressor,
which can do any of compressing, concatenating or renaming javascript files,
this may break Leaflet's own ability to determine its installed path, and in
turn break the method ``L.Icon.Default.imagePath()``.

To use Django's own knowledge of its static files to force this value
explicitly, use::

    'FORCE_IMAGE_PATH': True

Plugins
~~~~~~~

To ease the usage of plugins, django-leaflet allows specifying a set of plugins, that can
later be referred to from the template tags by name::

    'PLUGINS': {
        'name-of-plugin': {
            'css': ['relative/path/to/stylesheet.css', '/root/path/to/stylesheet.css'],
            'js': 'http://absolute-url.example.com/path/to/script.js',
            'auto-include': True,
        },
        . . .
    }

Both 'css' and 'js' support identical features for specifying resource URLs:

* can be either a plain string or a list of URLs
* each string can be:

  * absolute URL - will be included as-is; **example**: ``http://absolute-url.example.com/path/to/script.js``
  * a URL beginning from the root - will be included as-is;  **example**: ``/root/path/to/stylesheet.css``
  * a relative URL - settings.STATIC_URL will be prepended; **example**: ``relative/path/to/stylesheet.css`` will be included as **/static/relative/path/to/stylesheet.css** (depending on your setting for STATIC_URL)

Now, use ``leaflet_js`` and ``leaflet_css`` tags to load CSS and JS resources of
configured Leaflet plugins.

By default only plugins with ``'auto-include'`` as True will be included.

To include specific plugins in the page, specify plugin names, comma separated::

    {% load leaflet_tags %}

    <head>
        ...
        {% leaflet_js  plugins="bouncemarker,draw" %}
        {% leaflet_css plugins="bouncemarker,draw" %}
    </head>

To include all plugins configured in ``LEAFLET_CONFIG['PLUGINS']``, use::

    {% leaflet_js plugins="ALL" %}
    {% leaflet_css plugins="ALL" %}



Leaflet map forms widgets
-------------------------

A Leaflet widget is provided to edit geometry fields.
It embeds *Leaflet.draw* in version *0.2.2*.


.. image :: https://f.cloud.github.com/assets/546692/1048836/78b6ad94-1094-11e3-86d8-c3e88626a31d.png


In Adminsite
~~~~~~~~~~~~

::

    from django.contrib import admin
    from leaflet.admin import LeafletGeoAdmin

    from .models import WeatherStation


    admin.site.register(WeatherStation, LeafletGeoAdmin)


In forms
~~~~~~~~

With *Django* >= 1.6:

::

    from django import forms

    from leaflet.forms.widgets import LeafletWidget


    class WeatherStationForm(forms.ModelForm):

        class Meta:
            model = WeatherStation
            fields = ('name', 'geom')
            widgets = {'geom': LeafletWidget()}

With all *Django* versions:

::

    from django import forms

    from leaflet.forms.fields import PointField


    class WeatherStationForm(forms.ModelForm):
        geom = PointField()

        class Meta:
            model = WeatherStation
            fields = ('name', 'geom')

The related template would look like this:

::

    {% load leaflet_tags %}
    <html>
      <head>
       {% leaflet_js plugins="forms" %}
       {% leaflet_css plugins="forms" %}
      </head>
      <body>
        <h1>Edit {{ object }}</h1>
        <form action="POST">
            {{ form }}
            <input type="submit"/>
        </form>
      </body>
    </html>


Every map field will trigger an event you can use to add your custom machinery :

::

    map.on('map:loadfield', function (e) {
        ...
        // Customize map for field
        console.log(e.field, e.fieldid);
        ...
    });


If you need a reusable customization of widgets maps, first override the JavaScript field behaviour by extending ``L.GeometryField``, then in Django subclass the ``LeafletWidget`` to specify the custom ``geometry_field_class``.

::

    YourGeometryField = L.GeometryField.extend({
        addTo: function (map) {
            L.GeometryField.prototype.addTo.call(this, map);
            // Customize map for field
            console.log(this);
        },
        // See GeometryField source (static/leaflet/leaflet.forms.js) to override more stuff...
    });

::

    class YourMapWidget(LeafletWidget):
        geometry_field_class = 'YourGeometryField'

    class YourForm(forms.ModelForm):
        class Meta:
            model = YourModel
            fields = ('name', 'geom')
            widgets = {'geom': YourMapWidget()}

Plugins
~~~~~~~

It's possible to add extras JS/CSS or auto-include *forms* plugins
everywhere: ::

    LEAFLET_CONFIG = {
        'PLUGINS': {
            'forms': {
                'auto-include': True
            }
        }
    }

( *It will be merged over default minimal set required for edition* )


Details
~~~~~~~

* It relies on global settings for map initialization.
* It works with local map projections. But SRID is specified globally
  through ``LEAFLET_CONFIG['SRID']`` as described below.
* Javascript component for de/serializing fields value is pluggable.
* Javascript component for Leaflet.draw behaviour initialization is pluggable.



Advanced usage
--------------


``{% leaflet_map %}`` tag parameters
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

* ``callback``: javascript function name for initialization callback.
  (Default: None).

* ``fitextent``: control if map initial view shoud be set to extent setting.
  (Default: ``True``). Setting fixextent to ``False`` will prevent view reset
  and scale controls to be added.

* ``creatediv``: control if the leaflet map tags creates a new div or not.
  (Default: ``True``).
  Useful to put the javascript code in the header or footer instead of the
  body of the html document. If used, do not forget to create the div manually.

* ``loadevent``: One or more space-separated *window* events that trigger map initialization.
  (Default: ``load``, i.e. all page resources loaded).
  If empty values is provided, then map initialization is immediate.
  And with a wrong value, the map is never initialized. :)

* ``settings_overrides``: Map with overrides to the default LEAFLET_CONFIG settings.
  (Default: {}).

Config overrides
~~~~~~~~~~~~~~~~

It is possible to dynamically override settings in ``LeafletWidget`` init:

::

    from leaflet.forms.widgets import LeafletWidget


    class WeatherStationForm(forms.ModelForm):

        class Meta:
            model = WeatherStation
            fields = ('name', 'geom')
            widgets = {'geom': LeafletWidget(attrs={
                'settings_overrides': {
                    'DEFAULT_CENTER': (6.0, 45.0),
                }
            })}

For overriding the settings in ``LeafletGeoAdmin``, use set the appropriate property:

::

    class WeatherStationAdminAdmin(LeafletGeoAdmin):
        settings_overrides = {
           'DEFAULT_CENTER': (6.0, 45.0),
        }


Projection
~~~~~~~~~~

It is possible to setup the map spatial reference in ``LEAFLET_CONFIG``::

    'SRID': 2154  # See http://spatialreference.org

Additional parameter is required to compute scale levels : the tiles extent in
local projection::

    'TILES_EXTENT': [924861,6375196,985649,6448688],

For more information, `have a look at this example <http://blog.mathieu-leplatre.info/leaflet-tiles-in-lambert-93-projection-2154.html>`_.

Example of TileCache configuration compatible with Leaflet:

::

    [scan-portrait]
    type=WMSLayer
    layers=scan100,scan25
    url=http://server/wms?
    extension=jpg
    tms_type=google
    srs=EPSG:2154
    bbox=924861,6375196,985649,6448688

    [cache]
    type=GoogleDisk
    expire=2592000
    base=/tmp/tiles


By default, *django-leaflet* will try to load the spatial reference from your static
files at "proj4js/{{ srid }}.js". If it fails, it will eventually rely on
`<spatialreference.org>`_.


=========
TUTORIALS
=========

* `GeoDjango maps with Leaflet <http://blog.mathieu-leplatre.info/geodjango-maps-with-leaflet.html>`_


=======
AUTHORS
=======

* `Mathieu Leplatre <http://mathieu-leplatre.info>`_
* `Ariel Núñez <http://ingenieroariel.com>`_
* `Boris Chervenkov <https://github.com/boris-chervenkov>`_
* `Marco Badan <https://github.com/itbabu>`_
* `Bruno Renié <https://github.com/brutasse>`_
* `Simon Thépot <https://github.com/djcoin>`_
* `Thibault Jouannic <https://github.com/thibault>`_
* `jnm <https://github.com/jnm>`_
* `Manel Clos <https://github.com/manelclos>`_
* `Gaël Utard <https://github.com/gutard>`_
* `Alex Marandon <https://github.com/amarandon>`_
* `ollb <https://github.com/ollb>`_
* `smcoll <https://github.com/smcoll>`_
* `jnm <https://github.com/jnm>`_
* `OKso <https://github.com/oksome>`_
* `Florent Lebreton <https://github.com/fle/>`_
* `rgreenemun <https://github.com/rgreenemun>`_
* `Marco Badan <https://github.com/itbabu>`_
* David Martinez Morata
* `NotSqrt <https://github.com/NotSqrt>`_
* `Dylan Verheul <https://github.com/dyve>`_
* `Mactory <https://github.com/Mactory>`_
* `Petr Dlouhy <https://github.com/PetrDlouhy>`_

|makinacom|_

.. |makinacom| image:: http://depot.makina-corpus.org/public/logo.gif
.. _makinacom:  http://www.makina-corpus.com

=======
LICENSE
=======

* Lesser GNU Public License
* Leaflet Copyright - 2010-2011 CloudMade, Vladimir Agafonkin


=========
CHANGELOG
=========

0.18.0 (2016-01-04)
-------------------

**New features**

* Use a LazyEncoder to allow lazy translations in settings (#132, thanks @Mactory)
* Enable settings_overrides also for admin (fixes #120, thanks @PetrDiouhy)
* Add tests for Django 1.9 and Python 3.5 (thanks @itbabu)

**Bug fixes**

* Fix LeafletWidget behaviour on GeometryCollectionField (fixes #135)


0.17.1 (2015-12-16)
-------------------

* Update Leaflet to 0.7.7
* Update Leaflet-draw to 0.2.4
* Fix rendering of leaflet widget when initial value is an empty string


0.17.0 (2015-11-11)
-------------------

**New features**

* Pass relative URLs for static files through django.contrib.staticfiles (thanks @dyve, fixes #111)
* Allow to override settings at the template tag level (thanks @PetrDiouhy, fixes #59)
* Update Leaflet to 0.7.5 (@dyve)
* Add Czech locale (thanks @PetrDiouhy)

**Bug fixes**

* Fix interaction with django-geojson (#106, thanks @batisteo)
* Use protocol independant URLs in default OSM tiles (thanks @NotSqrt)
* Fix deprecated TEMPLATE_DEBUG (#121, thanks @josenaka)
* Fix errors with multi-word field names (#123, thanks @josemazo)
* Fix loadevent not being taken into account in forms (#127, thanks @josemazo)


0.16.0 (2015-04-17)
-------------------

**New features**

* Add setting ``FORCE_IMAGE_PATH`` to bypass Leaflet guess on image paths
  (*useful when using django-compressor*) (thanks @nimasmi)
* Add Hebrew translations (thanks @nonZero)
* Map attribution can be translated using ugettext_lazy

**Bug fixes**

* Fix widgets hanging forever with points (thanks @Azimkhan, fixes #90)
* Remove setTimeout when calling setView() (thanks @manelclos, fixes #89)
* Fix minZoom/maxZoom when undefined in settings (thanks Manel Clos)


0.15.2 (2014-12-22)
-------------------

* Allow to set any leaflet tileLayer option in ``TILES`` and ``OVERLAYS`` settings (fixes #70).


0.15.1 (2014-12-04)
-------------------

* Remove special characters in README (fixes #82)
* Fix translation in French (fixes #86)
* Fix es localization


0.15.0 (2014-10-24)
-------------------

* Add ability to add overlay tile layers via new setting ``OVERLAYS``.

0.14.2 (2014-10-24)
-------------------

* Fix Django 1.7 support in tests (thanks Marco Badan)
* Add spanish translations (thanks David Martinez)

0.14.1 (2014-07-30)
-------------------

* Fix draw events being received for each draw control on the map.
  (**Caution**: ``map.drawControl`` attribute is not set anymore)


0.14.0 (2014-07-29)
-------------------

* Fix GeoJSON serialization when creating new MultiPoint records
* Make the only layer match the map max/min_zoom (fixes #67) (thanks Manel Clos)
* Added widget attribute to edit several fields on the same map


0.13.7 (2014-06-26)
-------------------

* Fix typo in default proj4js path (ref #71)


0.13.6 (2014-06-26)
-------------------

* Setup Projection machinery in Leaflet forms if necessary
* Django Leaflet forms fiels without libgeos installed (thanks Florent Lebreton)


0.13.5 (2014-06-18)
-------------------

* Prevent SRID download when default is used


0.13.4 (2014-06-13)
-------------------

* Fix SRID projection file not being loaded


0.13.3 (2014-06-10)
-------------------

* Upgrade to Leaflet 0.7.3


0.13.2 (2014-04-15)
-------------------

* Fix regression where maps have null as max zoom


0.13.1 (2014-04-10)
-------------------

* Fix GEOS dependency, back as optional for geometry edition only (fixes #65)
* Add minZoom and maxZoom to map initialization
* Add support of advanced static files locations, like S3 (thanks @jnm)


0.13.0 (2014-03-26)
-------------------

* Add support of Leaflet form fields on Django >= 1.4.2 (thanks Gaël Utard)


0.12 (2014-03-22)
-----------------

* Add support of GeoJSON fields


0.11.1 (2014-02-12)
-------------------

* Do not complain about tile extent if SRID is 3857


0.11.0 (2014-02-07)
-------------------

* Add control of `metric` and `imperial` in `SCALE` option (thanks @smcoll)
* Upgrade to Leaflet.draw 0.2.3


0.10.1 (2014-02-03)
-------------------

* Upgrade to Leaflet 0.7.2


0.10.0 (2014-01-22)
-------------------

* Python 3 support (thanks @itbabu)
* Added JavaScript test using Mocha

0.9.0 (2013-12-11)
------------------

* Upgrade to Leaflet 0.7.1
* Fix unsaved warning being always triggered on Internet Explorer.
* Added DE locale (thanks @rosscdh)
* Fix installation with python 2.6 (thanks @ollb)


0.8.5 (2013-11-05)
------------------

* Fix name collision.


0.8.4 (2013-11-05)
------------------

* Fix regression in Django leaflet options serialization.


0.8.3 (2013-11-05)
------------------

* Switch to lazy gettext in leaflet module init.


0.8.2 (2013-10-31)
------------------

* Fix drawing of multi-polygon (fixes #37)
* Fix attached data for events with jQuery fallback (fixes #38)
* Fix Javascript syntax errors when using form prefixes (fixes #40)

0.8.1 (2013-09-30)
------------------

* Fix Leaflet library inclusion with "plugins=ALL" outside Admin.
* Do not include translations in every widgets outside Admin.
* Fix syntax error if form widget translations contains quotes.
* Fix dependency error if Leaflet is loaded after the form widget in the DOM.
* Respect plugins declaration order using OrderedDicts
* Prepend forms assets (instead of extend) if PLUGINS['forms'] already exists.

0.8.0 (2013-09-18)
------------------

* Renamed Leaflet map fragment template
* Leaflet map geometry widgets for adminsite and forms (requires Django 1.6)
* Fix geometry type restriction in form fields (fixes #32)
* Use jQuery for triggering events, only if CustomEvent constructor is not available (fixes #27, fixes #34)

0.7.4 (2013-08-28)
------------------

* Fix projection download error if not available
* Compute resolutions the same way TileCache does it, and provide
  example of TileCache configuration.
* Raise ImproperlyConfigured if TILES_EXTENT is not portrait (since not supported)

0.7.3 (2013-08-23)
------------------

* Do not use console() to warn about deprecated stuff if not available (<IE9)
* Fix apparence of Reset view control for Leaflet 0.6
* Add French and Italian locales

0.7.2 (2013-08-23)
------------------

* Fix JS error when no callback value is provided.


0.7.1 (2013-08-21)
------------------

* Fix map initialization with default tiles setting
* Fix map ``fitBounds()`` to ``SPATIAL_EXTENT`` in settings


0.7.0 (2013-08-21)
------------------

**Breaking changes**

* The ``leaflet_map`` template tag no longer registers initialization
  functions in global scope, and no longer adds map objects into ``window.maps``
  array by default. Use ``LEAFLET_CONFIG['NO_GLOBALS'] = False`` to
  restore these features.

* Initialization callback function no longer receives the map ``bounds`` in second
  argument, but the map options object.

**Deprecated**

* JS default callback function ( *<name>Init()* ) for map initialization is **deprecated**.
  Use explicit ``callback`` parameter in template tag,  or listen to window event ``map:init`` instead.
  (See *Use Leaflet API* section in README.)

* ``TILES_URL`` entry in ``LEAFLET_CONFIG`` is **deprecated**.
  Use ``TILES`` instead.

* Settings lookup is restricted to ``LEAFLET_CONFIG`` dict. Most notably,
  ``SRID``, ``MAP_SRID`` and ``SPATIAL_EXTENT`` at global Django settings level
  are discouraged.

**New features**

* Add ability to associate layers attributions from settings
* Add ``auto-include`` key for entries in ``PLUGINS`` setting, in order
  to implicity load plugins with ``leaflet_css`` and ``leaflet_js`` tags.
* Rewrote map initialization, into less flexible and obstruvise way.
* Use plugin system for Leaflet.MiniMap.
* Add ``loadevent`` parameter to ``leaflet_map`` tag.
* Map initialization is now idempotent, does nothing if map is already initialized.
* Add ``ATTRIBUTION_PREFIX`` setting to control prefix globally.


0.6.0 (2013-08-08)
------------------

* Upgrade to Leaflet 0.6.4

0.6.0a (2013-07-05)
-------------------

* Upgrade to Leaflet 0.6.2
* Upgrade Leaflet.Minimap (rev 3cd58f7)
* Upgrade Proj4Leaflet (rev f4f5b6d)

0.5.1 (2013-04-08)
------------------

* Add minimap support
* Drop Leaflet version switching
* Update Leaflet to 0.5.1
* Update Leaflet.Minimap
* Fix apparence of Reset view button

0.4.1 (2012-11-05)
------------------

* Fix div creation test in template.

0.4.0 (2012-11-05)
------------------

* Remove imperial scale.
* Add ``create_div`` parameter

0.3.0 (2012-10-26)
------------------

* Remove max resolution setting since it can be computed
* Allow scale control even if view is not set
* Upgrade Leaflet to 0.4.5

0.2.0 (2012-09-22)
------------------

* Fix packaging of templates
* Use template for <head> fragment
* Do not rely on spatialreference.org by default
* Default settings for SRID
* Default settings for map extent
* Default map height
* Default tiles base layer
* map variable is not global anymore

0.1.0 (2012-08-13)
------------------

* Initial support for map projection
* Show zoom scale by default
* Spatial extent configuration
* Initialization callback instead of global JS variable
* Leaflet version switching
* Global layers configuration

0.0.2 (2012-03-22)
------------------

* Add IE conditional CSS


0.0.1 (2012-03-16)
------------------

* Initial working version


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
