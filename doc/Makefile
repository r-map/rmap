docs:
	@echo Generating Project Documentation \($(TARGET)\)...
	@doxygen doxygen_doc.conf
	@mkdir -p doxygen_v4
	@doxygen doxygen_v4_master.conf
	@doxygen doxygen_v4_slave-mppt.conf
	@doxygen doxygen_v4_slave-radiation.conf
	@doxygen doxygen_v4_slave-rain.conf
	@doxygen doxygen_v4_slave-th.conf
	@doxygen doxygen_v4_slave-vwc.conf
	@doxygen doxygen_v4_slave-wind.conf
	@doxygen doxygen_v4_slave-leaf.conf
	@doxygen doxygen_v4_slave-level.conf
	@echo Documentation Generation Complete.
