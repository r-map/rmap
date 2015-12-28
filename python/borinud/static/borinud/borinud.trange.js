(function() {
    var mkdesc = function(root, p1, p2) {
        if (p1 == null && p2 == null)
            return root;
        if (p2 != null)
            root += " over " + p2 + " seconds";
        if (p1 == null)
            return root;
        if (p1 < 0)
            return root + "-" + p1 + " before reference time";
        return root + " at forecast time " + p1 + " seconds";
    };
    var describe = function(pind, p1, p2) {
        switch (pind)
        {
            case 0:   return mkdesc("Average", p1, p2);
            case 1:   return mkdesc("Accumulation", p1, p2);
            case 2:   return mkdesc("Maximum", p1, p2);
            case 3:   return mkdesc("Minimum", p1, p2);
            case 4:   return mkdesc("Difference (end minus beginning)", p1, p2);
            case 5:   return mkdesc("Root Mean Square", p1, p2);
            case 6:   return mkdesc("Standard Deviation", p1, p2);
            case 7:   return mkdesc("Covariance (temporal variance)", p1, p2);
            case 8:   return mkdesc("Difference (beginning minus end)", p1, p2);
            case 9:   return mkdesc("Ratio", p1, p2);
            case 51:  return mkdesc("Climatological Mean Value", p1, p2);
            case 200: return mkdesc("Vectorial mean", p1, p2);
            case 201: return mkdesc("Mode", p1, p2);
            case 202: return mkdesc("Standard deviation vectorial mean", p1, p2);
            case 203: return mkdesc("Vectorial maximum", p1, p2);
            case 204: return mkdesc("Vectorial minimum", p1, p2);
            case 205: return mkdesc("Product with a valid time ranging", p1, p2);
            case 254:
                if (p1 == 0 && p2 == 0) return "Analysis or observation, istantaneous value";
            else
                return "Forecast at t+" + p1 + " seconds, instantaneous value";
            default:  return pind + " " + p1 + " " + p2;
        }
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
            trange: {
                encode: encode,
                decode: decode,
                describe: describe
            }
        }
    });
}());
