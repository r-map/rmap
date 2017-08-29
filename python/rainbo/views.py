from django.shortcuts import render

# Create your views here.


def home(request):
    #current_site = get_current_site(request)
    return render(request, 'landing_page.html')

