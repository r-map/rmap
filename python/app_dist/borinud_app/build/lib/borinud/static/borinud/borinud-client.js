/**
 * borinud-client - web services client
 *
 * Copyright (C) 2013 ARPA-SIM <urpsim@smr.arpa.emr.it>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Author: Emanuele Di Giacomo <edigiacomo@arpa.emr.it>
*/
(function() {
    var borinud = {
        config: {
            root_el: null,
            ajax: {
                baseUrl: "borinud/v1/api/geojson",
                dataType: "json"
            },
        }
    };
    borinud.config.context = {
        encode: function(properties, ismobile) {
            var coords = properties.lon + "," + properties.lat;
            if (ismobile == true)
                coords = "*";

            return [
                properties.ident || "-",
                coords,
                properties.network,
                borinud.config.trange.encode(
                    properties.trange[0],
                    properties.trange[1],
                    properties.trange[2]),
                borinud.config.level.encode(
                    properties.level[0],
                    properties.level[1],
                    properties.level[2],
                    properties.level[3]),
                properties.var
            ].join("/")
        },
        describe: function(properties) {
            var b = borinud.config.B[properties.var];
            var l = borinud.config.level.describe(
                properties.level[0],
                properties.level[1],
                properties.level[2],
                properties.level[3]
            );
            var t = borinud.config.trange.describe(
                properties.trange[0],
                properties.trange[1],
                properties.trange[2]
            );

            var desc = b.description +
                " (" + b.unit + ")" +
                " - " + t +
                " - " + l +
                " (" + properties.network + ")";
            return desc;
        }
    };
    // Summary object
    borinud.summary = new function() {
        this.items = [];
        this.networks = [];
        this.vars = [];
        this.levels = [];
        this.tranges = [];
        this.var_contexts = [];
        // Update the summary
        // The "updated" event is triggered when the summary is updated
        this.update = function() {
            $this = this;
            $.ajax({
                url: borinud.config.ajax.baseUrl + "/*/*/*/*/*/*/summaries",
                dataType: borinud.config.ajax.dataType,
                success: function(resp) {
                    $this.items = resp.features;
                    // contexts
                    $this.var_contexts = _.chain(resp.features).map(function(val, idx) {
                        return {
                            "desc": borinud.config.context.describe(val.properties),
                            "id": borinud.config.context.encode(val.properties).split("/").slice(2).join("/"),
                        };
                    }).uniq(false, function(item) {
                        return item.id;
                    }).sortBy(function(item) {
                        return item.desc;
                    }).value();
                    // trigger event
                    $($this).trigger("updated");
                }
            });
        }
     };

    borinud.updateStations = function() {
        var v = borinud.config.root_el.find(".menu .var_contexts").val();
        var t = borinud.config.root_el.find(".menu .datetime").val();
        if (!v)
            return;
        var u = "/*/*/" + v + "/summaries/" + t;
        $.ajax({
            url: borinud.config.ajax.baseUrl + u,
            dataType: borinud.config.ajax.dataType,
            success: function(stations) {
                var f = new OpenLayers.Format.GeoJSON({
                    externalProjection: new OpenLayers.Projection("EPSG:4326"),
                    internalProjection: borinud.map.getProjectionObject()
                }).read(stations);
                var l = borinud.stationslayer;
                l.destroyFeatures();
                l.addFeatures(f);
                var b = l.getDataExtent();
                if (b)
                    borinud.map.zoomToExtent(b);
            }
        });
    };

    borinud.showFeatureTimeseries = function(e) {
        var f = e.feature;
        var t = borinud.config.root_el.find(".menu input.datetime").val();
        var u = borinud.config.context.encode(
            f.attributes,
            $("#ismobile").is(":checked") && f.attributes.ident != null
        ) + "/timeseries/" + t;
        $.ajax({
            url: borinud.config.ajax.baseUrl + "/"+ u,
            dataType: borinud.config.ajax.dataType,
            success: function(resp) {
                var dialog = $("<div>");
                var content = $("<div>").css("height", "80%");
                dialog.append(content);
                dialog.dialog({
                    title: borinud.config.context.describe(f.attributes),
                    modal: true,
                    height: borinud.config.root_el.height() * 0.9,
                    width: borinud.config.root_el.width() * 0.9,
                    close: function() {
                        $(this).remove();
                    }
                });
                var series = {
                    data: [],
                    label: borinud.config.B[f.attributes.var].unit
                };
                $.each(resp.features, function(idx, feature) {
                    series.data.push([
                                     new Date(feature.properties.date + "Z").getTime(),
                                     feature.properties.val
                    ]);
                });
                content.plot([series], {
                    xaxis: {
                        mode: "time",
                        timezone: "UTC",
                        minTickSize: [ 1, "hour" ]
                    },
                    series: {
                        lines: {
                            show: true
                        },
                        points: {
                            show: true
                        }
                    },
                    grid: {
                        hoverable: true
                    }
                });
                content.bind("plothover", function (event, pos, item) {
                    $("#plot-tooltip").remove();
                    if (item) {
                        var t = new Date(item.datapoint[0]).toJSON();
                        var v = item.datapoint[1];
                        $('<div id="plot-tooltip">' + t + ' - ' + v + '</div>').css({
                            "position": "absolute",
                            "display": "none",
                            "opacity": 0.80,
                            "top": item.pageY,
                            "left": item.pageX,
                            "padding": "0.5em",
                            "margin": "0",
                            "border": "0.3em solid silver",
                            "background-color": "white",
                            "z-index": 20000,
                            "font-size": "0.8em"
                        }).appendTo("body").fadeIn(200);
                    }
                });
            }
        });
    };

    borinud._createMenu = function() {
        var menu = $(".menu", borinud.config.root_el).empty();

        $("<select class='query var_contexts'>").appendTo(menu).html($.map(borinud.summary.var_contexts, function(obj, idx) {
            return "<option value='" + obj.id + "'>" + obj.desc + "</option>";
        }).join(""));

        $("<input class='query datetime'>").appendTo(menu).datepicker({
            dateFormat: 'yy/mm/dd',
            changeDay: true,
            changeMonth: true,
            changeYear: true,
            onSelect: function() {
                $(this).change();
            }
        }).datepicker('setDate', new Date());

        $("<input class='query' id='ismobile' type='checkbox'>").appendTo(menu);
        $("<label for='ismobile'>Stations are mobile</label>").appendTo(menu);
        
        // calculate min and max dates
        var daterange = _.reduce(this.summary.items, function(memo, f) {
            var d = memo;
            if (memo[0] == null || f.properties.date[0] < memo[0])
                d[0] = f.properties.date[0];
            if (memo[1] == null || f.properties.date[1] > memo[1])
                d[1] = f.properties.date[1];
            return d;
        }, [null, null]);
        if (daterange[0])
            $(".query.datetime").datepicker("option", "minDate", new Date(daterange[0]));
        if (daterange[1])
            $(".query.datetime").datepicker("option", "maxDate", new Date(daterange[1]));
            
        console.log(daterange);
        $(menu).trigger("change");
    };

    borinud.map = new OpenLayers.Map();
    borinud.stationslayer = new OpenLayers.Layer.Vector("stations");

    borinud._createMap = function() {
        borinud.map.addLayer(new OpenLayers.Layer.OSM());

        borinud.map.render(borinud.config.root_el.find(".map").get(0));
        borinud.map.zoomToMaxExtent();

        borinud.map.addLayer(borinud.stationslayer);

        $(".menu").change(function(e) {
            borinud.updateStations();
        });

        borinud.stationslayer.events.on({
            featureselected: borinud.showFeatureTimeseries
        });

        var selectStation = new OpenLayers.Control.SelectFeature(borinud.stationslayer);
        borinud.map.addControl(selectStation);
        selectStation.activate();
    };

    // Init function.
    borinud.init = function() {
        $(document).ajaxStart($.blockUI).ajaxStop($.unblockUI);
        if (borinud.config.root_el == null) {
            var msg = "borinud error: mandatory option borinud.config.root_el not set";
            alert(msg);
            throw new Error(msg);
        }
        borinud.config.root_el = $(borinud.config.root_el);
        borinud.config.root_el.append($("<div class='menu'/>").height("10%"));
        borinud.config.root_el.append($("<div class='map'/>").height("80%"));
        borinud.config.root_el.find(".map").css("position", "relative");

        $(borinud.summary).bind("updated", function() {
            borinud._createMenu();
        });
        borinud.summary.update();
        borinud.config.root_el.find(".map").height(function(i, h) {
            return window.innerHeight - $(this).offset().top;
        });
        borinud._createMap();
    };
    this.borinud = borinud;
}());
