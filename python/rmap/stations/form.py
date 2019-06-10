"""
Forms and validation code for user registration.
"""
from django.utils.translation import ugettext_lazy as _
from django import forms
#from registration.forms import RegistrationForm
from registration.forms import RegistrationFormTermsOfService
from django.core import validators
import re

class RmapRegistrationForm(RegistrationFormTermsOfService):
    """
    Custom form to limit user length and content
    """
    username = forms.CharField(label='Username',help_text=_('Required: max 9 lowercase alphanumeric characters'), max_length=9,
                               validators=[
                                   validators.RegexValidator(re.compile('^[a-z0-9_]+$'), _('Enter a valid username.'), 'invalid')
                               ])

