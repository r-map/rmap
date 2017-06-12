(function() {
    var describe_level = function(ltype, l1) {
        switch (ltype)
        {
            case 1:   return "Ground or water surface";
            case 2:   return "Cloud base level";
            case 3:   return "Level of cloud tops";
            case 4:   return "Level of 0°C isotherm";
            case 5:   return "Level of adiabatic condensation lifted from the surface";
            case 6:   return "Maximum wind level";
            case 7:   return "Tropopause";
            case 8:   if (l1 == 0)
                return "Nominal top of atmosphere";
            else
                return "Nominal top of atmosphere, channel " + l1;
            case 9:   return "Sea bottom";
            case 20:  return "Isothermal level, " + l1/10 + "K";
            case 100: return "Isobaric surface, " + l1/100 + "hPa";
            case 101: return "Mean sea level";
            case 102: return l1/1000 + "m above mean sea level";
            case 103: return l1/1000 + "m above ground";
            case 104: return "Sigma level " + l1/10000;
            case 105: return "Hybrid level " + l1;
            case 106: return l1/1000 + "m below land surface";
            case 107: return "Isentropic (theta) level, potential temperature " + l1/10 + "K";
            case 108: return "Pressure difference " + l1/100 + "hPa from ground to level";
            case 109: return "Potential vorticity surface " + l1/1000 + " 10-6 K m2 kg-1 s-1";
            case 111: return "ETA* level " + l1/10000;
            case 117: return "Mixed layer depth " + l1/1000 + "m";
            case 160: return l1/1000 + "m below sea level";
            case 200: return "Entire atmosphere (considered as a single layer)";
            case 201: return "Entire ocean (considered as a single layer)";
            case 204: return "Highest tropospheric freezing level";
            case 206: return "Grid scale cloud bottom level";
            case 207: return "Grid scale cloud top level";
            case 209: return "Boundary layer cloud bottom level";
            case 210: return "Boundary layer cloud top level";
            case 211: return "Boundary layer cloud layer";
            case 212: return "Low cloud bottom level";
            case 213: return "Low cloud top level";
            case 214: return "Low cloud layer";
            case 215: return "Cloud ceiling";
            case 220: return "Planetary Boundary Layer";
            case 222: return "Middle cloud bottom level";
            case 223: return "Middle cloud top level";
            case 224: return "Middle cloud layer";
            case 232: return "High cloud bottom level";
            case 233: return "High cloud top level";
            case 234: return "High cloud layer";
            case 235: return "Ocean Isotherm Level" + l1/10 + "m";
            case 240: return "Ocean Mixed Layer";
            case 242: return "Convective cloud bottom level";
            case 243: return "Convective cloud top level";
            case 244: return "Convective cloud layer";
            case 245: return "Lowest level of the wet bulb zero";
            case 246: return "Maximum equivalent potential temperature level";
            case 247: return "Equilibrium level";
            case 248: return "Shallow convective cloud bottom level";
            case 249: return "Shallow convective cloud top level";
            case 251: return "Deep convective cloud bottom level";
            case 252: return "Deep convective cloud top level";
            case 253: return "Lowest bottom level of supercooled liquid water layer";
            case 254: return "Highest top level of supercooled liquid water layer";
            case 255: return "Missing";
            case 256: return "Clouds";
            case 257: return "Information about the station that generated the data"
            case 258:
                switch (l1) {
                case 0: return "General cloud group";
                case 1: return "CL";
                case 2: return "CM";
                case 3: return "CH";
                default: return ltype1 + " " + l1;
            }
            break;
            case 259: return "Cloud group " + l1;
            case 260: return "Cloud drift group " + l1;
            case 261: return "Cloud elevation group " + l1;
            case 262: return "Direction and elevation of clouds";
            case null: return "-";
            default:    return ltype +" " + l1;
        }
    };
    var describe = function(ltype1, l1, ltype2, l2) {
        if (ltype2 == null || l2 == null)
            return describe_level(ltype1, l1);
        else if (ltype1 == 256)
            return describe_level(ltype1, l1) + ", " + describe_level(ltype2, l2);
        else
            return "Layer from [" + describe_level(ltype1, l1) + "] to [" + describe_level(ltype2, l2) + "]";
    };
    var encode = function(ltype1, l1, ltype2, l2) {
        var res = [];
        for (var idx in arguments) {
            if (arguments[idx] != null)
                res.push(arguments[idx]);
            else
                res.push("-");
        }
        return res.join(",");
    };
    var decode = function(str) {
        var res = [];
        var items = str.split(",");
        for (var idx in items)  {
            if (items[idx] == "-")
                res.push(null);
            else
                res.push(parseInt(items[idx]));
        }
        return res;
    };
    this.borinud = this.borinud || {};
    $.extend(true, this.borinud, {
        config: {
            level: {
                encode: encode,
                decode: decode,
                describe: describe
            }
        }
    });
}());
