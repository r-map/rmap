AM_CPPFLAGS = -I$(top_srcdir) -I$(top_builddir) -g

bin_PROGRAMS = mqtt2bufr bufr2mqtt storedjson2bufr

noinst_LTLIBRARIES = libmqtt2bufr-utils.la

libmqtt2bufr_utils_la_SOURCES = parser.cc

mqtt2bufr_SOURCES = mqtt2bufr.cc

mqtt2bufr_LDADD = libmqtt2bufr-utils.la

bufr2mqtt_SOURCES = bufr2mqtt.cc

bufr2mqtt_LDADD = libmqtt2bufr-utils.la

storedjson2bufr_SOURCES = storedjson2bufr.cc

storedjson2bufr_LDADD = libmqtt2bufr-utils.la

dist_man_MANS = mqtt2bufr.1 bufr2mqtt.1 storedjson2bufr.1

mqtt2bufr.1: mqtt2bufr.cc
	$(HELP2MAN) --no-info --name="MQTT subscriber for BUFR messages" --output=$@ ./mqtt2bufr

bufr2mqtt.1: bufr2mqtt.cc
	$(HELP2MAN) --no-info --name="Publish BUFR messages to MQTT" --output=$@ ./bufr2mqtt

storedjson2bufr.1: storedjson2bufr.cc
	$(HELP2MAN) --no-info --name="Convert stored JSON to generic BUFR" --output=$@ ./storedjson2bufr

EXTRA_DIST = \
	     parser.h mqtt2bufr.spec
