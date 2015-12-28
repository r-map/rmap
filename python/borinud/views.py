from django.shortcuts import render


def get_map(request):
    return render(request, 'borinud/map.html')
