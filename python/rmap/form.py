"""
Forms and validation code for user wizard.
"""
from django.utils.translation import ugettext_lazy as _
from django.core import validators
from django import forms
import re
import decimal
from stations.models import StationMetadata
import settings

def get_stations():
    stations=[]
    for station in StationMetadata.objects.all():
        stations.append((station.slug,station.name))
    return stations


class WizardForm(forms.Form):
    def __init__(self, *args, **kwargs):
        super(WizardForm, self).__init__(*args, **kwargs)
        self.fields['station'] = forms.ChoiceField(help_text=_('station to configure'),
                                                   choices=get_stations(),initial=settings.stationslug)
    

    username = forms.CharField(label=_('Your username'), max_length=9,validators=[
        validators.RegexValidator(re.compile('^[\w]+$'), _('Enter a valid username.'), 'invalid')
    ])

    password = forms.CharField(label='Password',help_text=_('Password'))

    latitude = forms.DecimalField(label='Latitude',help_text=_('Latitude'),min_value=decimal.Decimal("0."),max_value=decimal.Decimal("90."),decimal_places=5)
    longitude = forms.DecimalField(label='Longitude',help_text=_('Longitude'),min_value=decimal.Decimal("0."),max_value=decimal.Decimal("360."),decimal_places=5)


class WizardForm2(forms.Form):

    ssid = forms.CharField(label=_('SSID of your wiki network'),required=False)

    password = forms.CharField(label='Password',help_text=_('Password'),required=False)

