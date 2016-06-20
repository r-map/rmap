var AmatYr = function(apiurl) {
    this.apiurl = apiurl;
    this.current_weather;
    this.windroseData;
    this.that = this;
    that = this;
    var day = new Date();
    var today=day.getDate()
    var yesterday = day.getDate()-1;
    var month = day.getMonth()+1; //January is 0!

    // il mese deve essere del tipo 01, 02...... 10, 11, 12
    if(month<10){
      month='0'+month;
    }

    var year = day.getFullYear();


    currentsource = false;
    // array of all sparklines, used for updating them dynamically
    var sparklines = [];

    /* Initial and on resize we draw draw draw */
    on_resize(function() {
        // Redraw graphs
        draw(currentsource);
        // Redraw sparklines
        for(key in sparklines) {
            sparklines[key].redrawWithAnimation();
        }
    }); // these parenthesis does the trick

    // debulked onresize handler
    function on_resize(c,t){onresize=function(){clearTimeout(t);t=setTimeout(c,300)};return c};
/*
    // Fetch current weather
    d3.json(apiurl + 'now', function(json) {
        current_weather = {
            current: json[0]
        };
        rivets.bind(document.getElementById('current_weather'), current_weather);
    });
*/
    /****
     *
     *
     * WIND ROSE SECTION
     *
     *
     */

     /*
    var drawWindrose = function(startarg) {
        if (this.windroseData == undefined) {
            // Get all the wind history and draw two wind roses
            d3.json('/api/windhist?start='+startarg, function(data) {
                this.windroseData = data;
                windrose.drawBigWindrose(data, "#windrose", "Frequency by Direction");
                windrose.drawBigWindrose(data, "#windroseavg", "Average Speed by Direction");
            });
        }else{ // update existing with new data
            d3.json('/api/windhist?start='+startarg, function(data) {
                this.windroseData = data;
                windrose.updateWindVisDiagrams(data);
            })
        }
    }
    // Create windroses
    windrose = new WindRose();

*/

// In questa sezione creo i Grafici

    var initPath = function() {
        // Register HTML5 push state handler for navbar links
        $("#main_nav a").bind("click", function(event){
            event.preventDefault();
            // Fix active link
            $('.navbar li.active').removeClass('active');
            $(this).closest('li').addClass('active');

            // Call path state to handle the clicked link
            Path.history.pushState({}, "", $(this).attr("href"));
        });

        Path.map("/year/:year").to(function(){
          var valori=[];
          var giorno = this.params['year'];
          if(giorno==='2016'){
            giorno=""+year;
          }
          var temp_url='http://rmapv.rmap.cc/borinud/api/v2/-/1162264,4465378/locali/254,0,0/103,2000,-,-/B12101/timeseries/'+giorno;
          var press_url='http://rmapv.rmap.cc/borinud/api/v2/-/1162264,4465378/locali/254,0,0/1,-,-,-/B10004/timeseries/'+giorno;
          var humidity_url='http://rmapv.rmap.cc/borinud/api/v2/-/1162264,4465378/locali/254,0,0/103,2000,-,-/B13003/timeseries/'+giorno;
          var daily_rain='http://rmapv.rmap.cc/borinud/api/v2/-/1162264,4465378/locali/1,0,3600/1,-,-,-/B13011/timeseries/'+giorno;

          d3.json(temp_url, function(json) {
            for(var i=0;i<json.length;i++){
              var elem={};
              var item=json[i];
              elem.datetime=item.date;
              elem.outtemp=(item.data.vars[0].B12101)-273.15;
              valori.push(elem);
          }

          });

          d3.json(press_url, function(json) {
            for(var i=0;i<json.length;i++){
              var item=json[i];
              valori[i].barometer=(item.data.vars[0].B10004)/100;
            }
          });

          d3.json(humidity_url, function(json) {
            for(var i=0;i<json.length;i++){
              var item=json[i];
            valori[i].humidity=item.data.vars[0].B13003;
            }
          });

          d3.json(daily_rain, function(json) {
            for(var i=0;i<json.length;i++){
              var item=json[i];
            valori[i].dayrain=item.data.vars[0].B13011;
            }
          });
          currentsource=valori;
          // prendo il json che ho creato e lo do in pasto alla funzione draw che disegnerà il grafico
         draw(valori);

        });

        // grafico della sezione day/today e day/yesterday
        Path.map("/day/:day").to(function(){
            var valori=[];
            var giorno = this.params['day'];
            if(giorno==='today'){
              giorno=today
            }
            else {
              giorno=yesterday;
            }
            var temp_url='http://rmapv.rmap.cc/borinud/api/v2/-/1162264,4465378/locali/254,0,0/103,2000,-,-/B12101/timeseries/'+year+'/'+month+'/'+giorno;
            var press_url='http://rmapv.rmap.cc/borinud/api/v2/-/1162264,4465378/locali/254,0,0/1,-,-,-/B10004/timeseries/'+year+'/'+month+'/'+giorno;
            var humidity_url='http://rmapv.rmap.cc/borinud/api/v2/-/1162264,4465378/locali/254,0,0/103,2000,-,-/B13003/timeseries/'+year+'/'+month+'/'+giorno;
            var daily_rain='http://rmapv.rmap.cc/borinud/api/v2/-/1162264,4465378/locali/1,0,3600/1,-,-,-/B13011/timeseries/'+year+'/'+month+'/'+giorno;

            d3.json(temp_url, function(json) {
              for(var i=0;i<json.length;i++){
                var elem={};
                var item=json[i];
                elem.datetime=item.date;
                elem.outtemp=(item.data.vars[0].B12101)-273.15;
                valori.push(elem);
            }

            });

            d3.json(press_url, function(json) {
              for(var i=0;i<json.length;i++){
                var item=json[i];
                valori[i].barometer=(item.data.vars[0].B10004)/100;
              }
            });

            d3.json(humidity_url, function(json) {
              for(var i=0;i<json.length;i++){
                var item=json[i];
              valori[i].humidity=item.data.vars[0].B13003;
              }
            });

            d3.json(daily_rain, function(json) {
              for(var i=0;i<json.length;i++){
                var item=json[i];
              valori[i].dayrain=item.data.vars[0].B13011;
              }
            });
            currentsource=valori;
            // prendo il json che ho creato e lo do in pasto alla funzione draw che disegnerà il grafico
           draw(valori);

          //  drawWindrose(day);
        });

        //grafico della sezione hour/month
        Path.map("/hour/:arg").to(function(){
          var valori=[];
          var giorno = this.params['arg'];
          if(giorno==='month'){
            giorno=""+year+'/'+month;
          }
          var temp_url='http://rmapv.rmap.cc/borinud/api/v2/-/1162264,4465378/locali/254,0,0/103,2000,-,-/B12101/timeseries/'+giorno;
          var press_url='http://rmapv.rmap.cc/borinud/api/v2/-/1162264,4465378/locali/254,0,0/1,-,-,-/B10004/timeseries/'+giorno;
          var humidity_url='http://rmapv.rmap.cc/borinud/api/v2/-/1162264,4465378/locali/254,0,0/103,2000,-,-/B13003/timeseries/'+giorno;
          var daily_rain='http://rmapv.rmap.cc/borinud/api/v2/-/1162264,4465378/locali/1,0,3600/1,-,-,-/B13011/timeseries/'+giorno;

          d3.json(temp_url, function(json) {
            for(var i=0;i<json.length;i++){
              var elem={};
              var item=json[i];
              elem.datetime=item.date;
              elem.outtemp=(item.data.vars[0].B12101)-273.15;
              valori.push(elem);
          }

          });

          d3.json(press_url, function(json) {
            for(var i=0;i<json.length;i++){
              var item=json[i];
              valori[i].barometer=(item.data.vars[0].B10004)/100;
            }
          });

          d3.json(humidity_url, function(json) {
            for(var i=0;i<json.length;i++){
              var item=json[i];
            valori[i].humidity=item.data.vars[0].B13003;
            }
          });

          d3.json(daily_rain, function(json) {
            for(var i=0;i<json.length;i++){
              var item=json[i];
            valori[i].dayrain=item.data.vars[0].B13011;
            }
          });
          currentsource=valori;
          // prendo il json che ho creato e lo do in pasto alla funzione draw che disegnerà il grafico
         draw(valori);

        //    drawWindrose(arg);
        });
/*
        // sezione del sito dei 3 giorni, non ha url, è una sorta di home

        Path.map("/").to(function(){
            var width = $('#main').css('width').split('px')[0];
            d3.json(apiurl+'hour?start=3day', function(json) {
                // Save to global for redrawing
                currentsource = json;
                draw(json);
            });
            drawWindrose('3DAY');
        });
*/
        // Start listening for URL events
        Path.history.listen(true);  // Yes, please fall back to hashtags if HTML5 is not supported.

        // Initial fetch and draw
        Path.history.pushState({}, "", window.location.pathname);
        // Fix initial active link in case user got direct link and not started at /
        $('#main_nav a[href="'+window.location.pathname+'"]').closest('li').addClass('active');
    }
    initPath();

    // Fetch current weather with 1 minute interval
    setInterval(function() { d3.json(apiurl + 'now', function(json) {
        // Update each key with new val
        for(key in current_weather.current) {
            current_weather.current[key] = json[0][key]
        }
    }) }, 60*1000);

    // Draw sparkline
    d3.json(apiurl+'recent', function(json) {
        var tdata = [];
        var wdata = [];
        var pdata = [];
        json.forEach(function(k, v) {
            // Ordered wrong way, so unshift
            tdata.unshift(k.outtemp);
            wdata.unshift(k.windspeed);
            pdata.unshift(k.barometer);
        });
        sparklines.push(new sparkline('#sparkline', tdata, 'outtemp', tdata.length, 38, 'basis', true, 1000));
        sparklines.push(new sparkline('#windsparkline', wdata, 'windspeed', wdata.length, 38, 'basis', true, 1000));
        sparklines.push(new sparkline('#pressuresparkline', pdata, 'barometer', pdata.length, 38, 'basis', true, 1000));
        for(key in sparklines) {
            // Update each minute
            setInterval(sparklines[key].update, 60*1000);
        }
    });

    /****
     *
     *
     * RECORD WEATHER SECTION
     *
     *
     * */

    // Init the data structure to be used by records and rivets

    // Questa è la tabella in basso con tutti i dati
    var record_weather = {current:{}};

    // Fetch record weather
    var updateRecordsYear = function(year) {
        var keys = ['max', 'min'];
        var vals = ['outtemp', 'windspeed', 'dayrain', 'barometer', 'windgust', 'heatindex', 'windchill'];
        vals.forEach(function(k, v) {
            keys.forEach(function(func, idx) {
                // set key for rivets to set up proper setters and getters
                record_weather.current[func+k+'date'] = '';
                record_weather.current[func+k+'value'] = '';
                record_weather.current[func+k+'age'] = '';
                /// XX needs a black list for certain types that doesn't make sense
                // like min daily_rain or min windspeed
                d3.json(apiurl + 'record/'+k+'/'+func+'?start='+year, function(json) {
                    if (json) {
                        record_weather.current[func+k+'date'] = json[0].datetime;
                        record_weather.current[func+k+'value'] = json[0][k];
                        record_weather.current[func+k+'age'] = json[0].age;
                    }
                });
            })
        });
        // Additional entries
        var k = 'rain',
            func = 'sum';
        d3.json(apiurl + 'record/'+k+'/'+func+'?start='+year, function(json) {
            record_weather.current[func+k+'value'] = json[0][k];
        });
    }
    // initial structure for rivets to work
    updateRecordsYear('today');
    //rivets.bind(document.getElementById('r2013'), record_weather);

    // Populate all the tab content with the tab template
    d3.selectAll('#record_weather .tab-pane').forEach(function (tab) {
       $(tab).html($('#record_table_template').html());
    });
    // Bind rivets after both elemend and var is available and populated
    rivets.bind(document.getElementById('record_weather'), record_weather);

    // Bind the tab clicks to fetching new content
    $('#record_weather .nav-tabs a').bind('click', function(e) {
        e.preventDefault();

        // Find target container
        var tab = $($(this).attr('href'));

        // Get the year clicked
        var year = $(this).text();
        // Fetch new data from db
        updateRecordsYear(year);

        // Show tab
        $(this).tab('show');

    });





    /* Calendar generation */
    var updateCalender = function(json, year) {
        // TODO dynamic
        var width = $('#main').css('width').split('px')[0];

        // Format data to how calheatmap likes it
        var data = d3.nest()
          .key(function(d) { return d.date; })
          .rollup(function(d) { return d[0].dayrain; })
          .map(json);

        var cal = new CalHeatMap();

        // Remove spinner
        $('#cal-heatmap .spinner').remove();
        // Remove existing heatmaps
        $('#cal-heatmap svg').remove();
        // Draw calendar
        var range = 12; // Default 12 months
        if (year == new Date().getFullYear()) {
            range = new Date().getMonth()+1;
        }
        cal.init({
            data: data,
            itemSelector : "#cal-heatmap",
            domain : "month",       // Group data by month
            subDomain : "day",      // Split each month by days
            highlight : "now",
            itemName : ['mm', 'mm'],
            start : new Date(year, 0),
            end : new Date(year, 11),
            range : range,
            weekStartOnMonday: true,
            legend: [1, 4, 6, 8]    // Custom threshold for the scale
        });
    }

    var updateTabularData = function(data) {
        // Remove spinner
        $('#tabular .spinner').remove();

        var parseDate = d3.time.format("%Y-%m-%d %H:%M:%S").parse;

        // Add d3 js date for each datum
        data.forEach(function(d) {
            d.jsdate = parseDate(d.datetime);
        });

        // Group data by month
        var monthdata = d3.nest()
          .key(function(d) { return d.jsdate.getMonth(); })
          .map(data);

        var aggdata =  d3.nest()
          .key(function(d) { return d.jsdate.getMonth(); })
          .rollup(function(d) {
            return {
                barometer: d3.mean(d, function(g) { return +g.barometer }),
                windspeed: d3.mean(d, function(g) { return +g.windspeed }),
                windgustmax: d3.max(d, function(g) { return +g.windgust}),
                outtemp: d3.mean(d, function(g) { return +g.outtemp }),
                tempmax: d3.max(d, function(g) { return +g.tempmax }),
                tempmin: d3.min(d, function(g) { return +g.tempmin }),
                dayrain: d3.sum(d, function(g) { return +g.dayrain }),
                date: d3.min(d, function(g) { return +g.jsdate })
            }
          })
          .entries(data)

        var columns = ['datetime', 'dayrain', 'outtemp', 'windspeed', 'winddir'];
        // Create a collapsible group for each month
        aggdata.forEach(function (agg, i) {
            //console.log(agg, agg.values, agg.values.outtemp);
            var month = agg.key,
                values = agg.values,
                monthname = d3.time.format('%Y %B')(new Date(values.date)),
                header = monthname,
                headertext =
                    ' Rain: <span class="blue">' + amatyrlib.autoformat('dayrain', values.dayrain) + '</span>' +
                    ' Avg Temp: ' + amatyrlib.autoformat('outtemp', values.outtemp) +
                    ' Min Temp: ' + amatyrlib.autoformat('outtemp', values.tempmin) +
                    ' Max Temp: ' + amatyrlib.autoformat('outtemp', values.tempmax) +
                    ' Avg Wind: ' + amatyrlib.autoformat('windspeed', values.windspeed) +
                    ' Max Windgust: ' + amatyrlib.autoformat('windspeed', values.windgustmax)
                ,
                panel = d3.select('#accordion').append('div')
                .classed('panel', true),
            heading = panel.append('div')
                .classed('panel-heading', true),
            h4 = heading.append('h4')
                .html('<a data-toggle="collapse" data-parent="#accordion" href="#collapse-'+i+'">'+header+'</a><small>'+headertext+'</small>'),
            collapse = panel.append('div')
                .attr('id', 'collapse-'+i)
                .classed('panel-collapse collapse', true),
            body = collapse.append('div')
                .classed('panel-body', true),
            table = body.append('table')
                .classed('table table-condensed', true),
            thead = table.append("thead"),
            tbody = table.append("tbody");


            // append the header row
            thead.append("tr")
                .selectAll("th")
                .data(columns)
                .enter()
                .append("th")
                .text(function(column) { return column; });

            // create a row for each object in the data
            var rows = tbody.selectAll("tr")
                .data(monthdata[month])
                .enter()
                .append("tr");

            // create a cell in each row for each column
            var cells = rows.selectAll("td")
                .data(function(row) {
                    return columns.map(function(column) {
                        return {column: column, value: row[column]};
                    });
                })
            .enter()
                .append("td")
                .attr("style", "font-family: monospace")
                .html(function(d) { return amatyrlib.autoformat(d.column, d.value); });
        });
    }
    function getAndDrawYearData(year) {
        // Get the year data and use it for both calendar and tabular data
        d3.json(apiurl + 'year/'+year, function(json) {
            var parseDate = d3.time.format("%Y-%m-%d %H:%M:%S").parse;

            // Add d3 js date for each datum
            json.forEach(function(d) {
                d.date = ""+(+parseDate(d.datetime))/1000;
            });

            updateTabularData(json);
            // The tabular data uses the same data source as calendar, so it is
            // reused in that function
            updateCalender(json, year);
        });
    }
    // Draw current year
    getAndDrawYearData(new Date().getFullYear());


    // Auto update webcam
    setInterval(function () {
      var imgElem = document.getElementById("webcam");
      var newImg = new Image();
      newImg.src = imgElem.src.split("?")[0] + "?" + new Date().getTime();
      newImg.addEventListener("load", function (evt) {
        imgElem.src = newImg.src;
      });
    }, 30000);

    return this;
}
var amatyrlib = new amatyrlib();
var amatyr = new AmatYr('http://yr.hveem.no/api/');
