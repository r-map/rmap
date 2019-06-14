SECRET_KEY = 'fake-key'
INSTALLED_APPS = [
    'borinud',
]
DATABASES = {
    'default': {
        'ENGINE': 'django.db.backends.sqlite3',
        'NAME': ":memory:"
    },
}
