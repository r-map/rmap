function scan(msg, md)
    local area = bufr_read_area_fixed(msg)
    local proddef = bufr_read_proddef(msg)
    local report = msg:find("rep_memo")
    if report == "mobile"
    then
        area = bufr_read_area_mobile(msg)
    end
    if area then md:set(arki_area.grib(area)) end
    if proddef then md:set(arki_proddef.grib(proddef)) end
    if report then md:set(md.product:addValues{t=report:enqc()}) end
end
