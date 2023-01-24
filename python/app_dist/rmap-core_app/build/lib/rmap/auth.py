import hashlib
from django.contrib.auth.hashers import PBKDF2PasswordHasher

class PBKDF2SHA512PasswordHasher(PBKDF2PasswordHasher):
    """
    Alternate PBKDF2 hasher which uses SHA512 instead of SHA256.

    Note: As of Django 1.4.3, django.contrib.auth.models.User defines password 
    with max_length=128

    Our superclass (PBKDF2PasswordHasher) generates the entry for that field
    using the following format (see 
    https://github.com/django/django/blob/1.4.3/django/contrib/auth/hashers.py#L187):

        "%s$%d$%s$%s" % (self.algorithm, iterations, salt, hash)

    The lengths of the various bits in that format are:

     13  self.algorithm ("pbkdf2_sha512")
      5  iterations ("10000" - inherited from superclass)
     12  salt (generated using django.utils.crypto.get_random_string())
     89  hash (see below)
      3  length of the three '$' separators
    ---
    122  TOTAL

    122 <= 128, so we're all good.

    NOTES

    hash is the base-64 encoded output of django.utils.crypto.pbkdf2(password, salt, 
    iterations, digest=hashlib.sha512), which is 89 characters according to my tests.

        >>> import hashlib
        >>> from django.utils.crypto import pbkdf2
        >>> len(pbkdf2('t0ps3kr1t', 'saltsaltsalt', 10000, 0, hashlib.sha512).encode('base64').strip())
        89

    It's feasible that future versions of Django will increase the number of iterations 
    (but we only lose one character per power-of-ten increase), or the salt length. That 
    will cause problems if it leads to a password string longer than 128 characters, but 
    let's worry about that when it happens.
    """

    algorithm = "pbkdf2_sha512"
    digest = hashlib.sha512
    #salt_entropy = 12*8
    salt_entropy = 0
    
