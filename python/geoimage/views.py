from models import GeorefencedImage
from django.shortcuts import render
from django import forms
from datetime import date,datetime,timedelta,time
from widgets import MySelectDateWidget
from django.utils.translation import ugettext_lazy

class ExtremeForm(forms.Form):

    initial_start=date.today()-timedelta(days=10)
    initial_end=date.today()

    datetime_start = forms.DateTimeField(required=True,initial=initial_start,widget=MySelectDateWidget(),label=ugettext_lazy("Starting date"),help_text=ugettext_lazy("Elaborate starting from this date"))

    datetime_end = forms.DateTimeField(required=True,initial=initial_end,widget=MySelectDateWidget(),label=ugettext_lazy("Ending date"),help_text=ugettext_lazy("Elaborate ending to this date"))



def showImage(request,ident=None):

    if request.method == 'POST': # If the form has been submitted...
        form = ExtremeForm(request.POST) # A form bound to the POST data
        if form.is_valid(): # All validation rules pass

            datetime_start=form.cleaned_data['datetime_start']
            datetime_end=form.cleaned_data['datetime_end']

            datetime_start = datetime.combine(datetime_start.date(),time(00,00))
            datetime_end = datetime.combine(datetime_end.date(),time(23,59,59))

    else:

        now=datetime.utcnow()
        datetime_start=(now-timedelta(days=10))
        datetime_end=now
        form = ExtremeForm() # An unbound form

    if ident is None:
        print "query no ident:",datetime_start,datetime_end
        grimages=GeorefencedImage.objects.filter(date__gte=datetime_start,date__lte=datetime_end).order_by("date")
    else:
        print "query:",datetime_start,datetime_end,ident
        grimages=GeorefencedImage.objects.filter(date__gte=datetime_start,date__lte=datetime_end,ident__username=ident).order_by("date")

    return render(request, 'geoimage/georefencedimage_list.html',{'form': form,"grimages":grimages,"ident":ident})


def showOneImage(request,ident,id):
    grimage=GeorefencedImage.objects.get(ident__username=ident,id=id)
    print "grimage"
    print grimage
    return render(request, 'geoimage/georefencedimage.html',{"grimage":grimage})

