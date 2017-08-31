var today = new Date();
var dd = today.getDate();
var mm = today.getMonth()+1; //January is 0!
var yyyy = today.getFullYear();

if(dd<10) {
    dd = '0'+dd
} 

if(mm<10) {
    mm = '0'+mm
} 

today = yyyy + '/' + mm + '/' + dd;

$('#rain_spatial_series').attr('href', '/showdata/*/*/*/254,0,0/1,-,-,-/B20003/spatialseries/'+today+'?type=180');
$('#thunderstorm_spatial_series').attr('href', '/showdata/*/*/*/254,0,0/1,-,-,-/B20003/spatialseries/'+today+'?type=190');
$('#tornado_spatial_series').attr('href', '/showdata/*/*/*/254,0,0/1,-,-,-/B20003/spatialseries/'+today+'?type=199');
$('#visibility_spatial_series').attr('href', '/showdata/*/*/*/254,0,0/1,-,-,-/B20003/spatialseries/'+today+'?type=110');
$('#damage_spatial_series').attr('href', '/showdata/*/*/*/254,0,0/1,-,-,-/------/spatialseries/'+today);
$('#snow_spatial_series').attr('href', '/showdata/*/*/*/254,0,0/1,-,-,-/B20003/spatialseries/'+today+'?type=180');

