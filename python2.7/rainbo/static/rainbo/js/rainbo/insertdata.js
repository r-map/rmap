/* select only one type of weather */

$('input:radio').change(function(){
    $('input:radio').each(function(){
      this.checked = false;
      $(this).parent().removeClass();
    })
    this.checked = true;
    switch($(this).attr('id')) {
      case 'id_impact_detected_0':
            $(this).parent().addClass("id_impact_detected_0_selected");
            break;
      case 'id_impact_detected_1':
            $(this).parent().addClass("id_impact_detected_1_selected");
            break;
      case 'id_impact_detected_2':
            $(this).parent().addClass("id_impact_detected_2_selected");
            break;
      case 'id_impact_detected_3':
            $(this).parent().addClass("id_impact_detected_3_selected");
            break;
      case 'id_not_significant_0':
            $(this).parent().addClass("id_not_significant_0_selected");
            break;
      case 'id_visibility_intensity_0':
            $(this).parent().addClass("id_visibility_intensity_0_selected");
            break;
      case 'id_visibility_intensity_1':
            $(this).parent().addClass("id_visibility_intensity_1_selected");
            break;
      case 'id_snow_intensity_0':
            $(this).parent().addClass("id_snow_intensity_0_selected");
            break;
      case 'id_snow_intensity_1':
            $(this).parent().addClass("id_snow_intensity_1_selected");
            break;
      case 'id_snow_intensity_2':
            $(this).parent().addClass("id_snow_intensity_2_selected");
            break;
     case 'id_rain_intensity_0':
            $(this).parent().addClass("id_rain_intensity_0_selected");
            break;
      case 'id_rain_intensity_1':
            $(this).parent().addClass("id_rain_intensity_1_selected");
            break;
      case 'id_rain_intensity_2':
            $(this).parent().addClass("id_rain_intensity_2_selected");
            break;
      case 'id_rain_intensity_3':
            $(this).parent().addClass("id_rain_intensity_3_selected");
            break;
      case 'id_thunderstorm_intensity_0':
            $(this).parent().addClass("id_thunderstorm_intensity_0_selected");
            break;
      case 'id_thunderstorm_intensity_1':
            $(this).parent().addClass("id_thunderstorm_intensity_1_selected");
            break;
      case 'id_thunderstorm_intensity_2':
            $(this).parent().addClass("id_thunderstorm_intensity_2_selected");
            break;
      case 'id_thunderstorm_intensity_3':
            $(this).parent().addClass("id_thunderstorm_intensity_3_selected");
            break;
      case 'id_tornado_0':
            $(this).parent().addClass("id_tornado_0_selected");
            break;
   }
});
