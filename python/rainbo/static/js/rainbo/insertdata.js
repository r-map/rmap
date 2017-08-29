/* select only one type of weather */

$('input:radio').change(function(){
    $('input:radio').each(function(){
      this.checked = false;
      $(this).parent().removeClass("selected");
    })
    this.checked = true;
    $(this).parent().addClass("selected");
});
