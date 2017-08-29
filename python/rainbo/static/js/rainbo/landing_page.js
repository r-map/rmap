/*!
 * Start Bootstrap - Freelancer Bootstrap Theme (http://startbootstrap.com)
 * Code licensed under the Apache License v2.0.
 * For details, see http://www.apache.org/licenses/LICENSE-2.0.
 */

// jQuery for page scrolling feature
$(function() {
    $('body').on('click', '.page-scroll a', function(event) {
        var $anchor = $(this);
        $('html, body').stop().animate({
            scrollTop: $($anchor.attr('href')).offset().top
        }, 1000 );
        event.preventDefault();
    });
});

// Floating label headings for the contact form
$(function() {
    $("body").on("input propertychange", ".floating-label-form-group", function(e) {
        $(this).toggleClass("floating-label-form-group-with-value", !! $(e.target).val());
    }).on("focus", ".floating-label-form-group", function() {
        $(this).addClass("floating-label-form-group-with-focus");
    }).on("blur", ".floating-label-form-group", function() {
        $(this).removeClass("floating-label-form-group-with-focus");
    });
});

// Highlight the top nav as scrolling occurs
$('body').scrollspy({
    target: '.navbar-fixed-top'
})

// Closes the Responsive Menu on Menu Item Click
$('.navbar-collapse ul li a').click(function() {
    $('.navbar-toggle:visible').click();
});


//Script to Activate the Carousel
$('.carousel').carousel({
    interval: 5000 //changes the speed
})




$(window).scroll(function(){
    var changeHeaderOn = 300;
    if( $(document).scrollTop() > changeHeaderOn )
    {
        if( !$('.navbar-fixed-top').hasClass( 'navbar-shrink' ) )
        {
            $('.navbar-fixed-top').addClass( "navbar-shrink", 1000 );
        }
    }
    else{
        if( $('.navbar-fixed-top').hasClass( 'navbar-shrink' ) ){
            $('.navbar-fixed-top').removeClass( "navbar-shrink", 1000 );
        }  
    }
});


$(document).ready( function(){
    //init popover
    $('[data-toggle="popover"]').attr( "title", "");
    $('[data-toggle="popover"]').data( "placement", "top");
    $('[data-toggle="popover"]').data( "toggle"   , "popover" ) 
    $('[data-toggle="popover"]').data( "container", "body" )
    $('[data-toggle="popover"]').data( "trigger"  , "hover" )
    
    $('[data-toggle="popover"]').popover();
})