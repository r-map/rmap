/** @preserve
 * Author:        Moris Pozzati <pozzati@meeo.it> 
 * Description:   Include and initialize mea plugin        
 * Changelog:     Fri Nov 29 10:19:11 CET 2013
 *                First Version
 *
 *                Tue Sep 23 11:36:06 CEST 2014
 *                New Array.prototype.remove and Array.prototype.last
 *
 *                Thu Jan 15 16:44:41 CET 2015
 *                New String.prototype.startsWith and String.prototype.endsWith
 *
 *                Wed Oct 25 10:54:53 CET 2016
 *                integration on rainbo
 */


//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
//                   main javascript 
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
(function( window, undefined ) {
    rainbo = {
        __init__ : function(){

            $.each( rainbo, function(key, module){
                try{
                    if ( typeof module === "object" && typeof module.__init__ == 'function' ){
                        module.__init__();
                    }    
                } catch( err ){
                    console.log( err );
                    console.log( "Module " + key + " doesn't provide a valid __init__ method" );    
                }
            })

            //setup ajax csrf token
            $.ajaxSetup({
                beforeSend: function(xhr, settings) {
                    if ( !rainbo.csrfSafeMethod( settings.type ) && !this.crossDomain ) {
                        xhr.setRequestHeader("X-CSRFToken", rainbo.getCookie('csrftoken'));
                    }
                }
            });
        },
       
        csrfSafeMethod: function( method ){
            
            // these HTTP methods do not require CSRF protection
            return (/^(GET|HEAD|OPTIONS|TRACE)$/.test(method));
        },
        
        getCookie: function(name) {
            var cookieValue = null;
            if (document.cookie && document.cookie != '') {
                var cookies = document.cookie.split(';');
                for (var i = 0; i < cookies.length; i++) {
                    var cookie = jQuery.trim(cookies[i]);
                    // Does this cookie string begin with the name we want?
                    if (cookie.substring(0, name.length + 1) == (name + '=')) {
                        cookieValue = decodeURIComponent(cookie.substring(name.length + 1));
                        break;
                    }
                }
            }
            return cookieValue;
        }
        
   }
    
    window.rainbo = window.$$ = rainbo;
})( window );
