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

    password = forms.CharField(label=_('Password'),help_text=_('Password'))

    stationname = forms.CharField(label=_('Station name'),help_text=_('Station name'))

    latitude = forms.DecimalField(label=_('Latitude'),help_text=_('Latitude'),min_value=decimal.Decimal("0."),max_value=decimal.Decimal("90."),decimal_places=5)
    longitude = forms.DecimalField(label=_('Longitude'),help_text=_('Longitude'),min_value=decimal.Decimal("0."),max_value=decimal.Decimal("360."),decimal_places=5)

    height = forms.DecimalField(label=_('Station height (m.)'),help_text=_('Station height (m.)'),min_value=decimal.Decimal("-10."),max_value=decimal.Decimal("10000."),decimal_places=1)


class WizardForm2(forms.Form):

    ssid = forms.CharField(label=_('SSID of your wifi network'),required=False)

    password = forms.CharField(label='Password',help_text=_('Password'),required=False)

