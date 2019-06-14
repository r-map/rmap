AM_CPPFLAGS = -I$(top_srcdir) -I$(top_builddir) -g

bin_PROGRAMS = storedjson2bufr

bin_SCRIPTS = mqtt2bufr bufr2mqtt

noinst_LTLIBRARIES = libmqtt2bufr-utils.la

libmqtt2bufr_utils_la_SOURCES = parser.cc

storedjson2bufr_SOURCES = storedjson2bufr.cc

storedjson2bufr_LDADD = libmqtt2bufr-utils.la

man_MANS = mqtt2bufr.1 bufr2mqtt.1 storedjson2bufr.1


mqtt2bufr: mqtt2bufr.py
	sed \
		-e 's,[@]PACKAGE_BUGREPORT@,@PACKAGE_BUGREPORT@,g' \
		-e 's,[@]PACKAGE_VERSION@,@PACKAGE_VERSION@,g' \
		$< > $@
	chmod ugo+x $@

bufr2mqtt: bufr2mqtt.py
	sed \
		-e 's,[@]PACKAGE_BUGREPORT@,@PACKAGE_BUGREPORT@,g' \
		-e 's,[@]PACKAGE_VERSION@,@PACKAGE_VERSION@,g' \
		$< > $@
	chmod ugo+x $@


mqtt2bufr.1: mqtt2bufr
	$(HELP2MAN) --no-info --name="MQTT subscriber for BUFR messages" --output=$@ ./mqtt2bufr

bufr2mqtt.1: bufr2mqtt
	$(HELP2MAN) --no-info --name="Publish BUFR messages to MQTT" --output=$@ ./bufr2mqtt

storedjson2bufr.1: storedjson2bufr.cc
	$(HELP2MAN) --no-info --name="Convert stored JSON to generic BUFR" --output=$@ ./storedjson2bufr


check-local:
	python3 test_mqtt2bufr.py

EXTRA_DIST = \
			 parser.h mqtt2bufr.spec mqtt2bufr.py bufr2mqtt.py test_mqtt2bufr.py

CLEANFILES = mqtt2bufr bufr2mqtt
