from django.views.generic import ListView
from django.views.generic.detail import DetailView
from .models import Ticket,TicketImage,TicketAttachment
from django.shortcuts import render
from django import forms
from django.contrib.auth.decorators import login_required
from django.http import HttpResponse
from rmap import rmap_core
from django.core.paginator import Paginator
from django.shortcuts import get_object_or_404
from django.db.models import Q


def tickets_list(request,user=None,slug=None):

    if (user is None):
        query_user=Q()
    else:
        query_user = Q(stationmetadata__user__username=user) 

    if (slug is None):
        query_slug=Q()
    else:
        query_slug = Q(stationmetadata__slug=slug)


    if 'search' in request.GET:
        search = request.GET['search']
        query =  (Q(assigned_to__username__icontains=search) | Q(abstract__icontains=search) | Q(stationmetadata__slug__icontains=search))
    else:
        query = Q()
        search = None

    query = query & query_user & query_slug
                         
    mytickets = Ticket.objects.filter(query)

    paginator = Paginator(mytickets, 25) # Show 25 contacts per page.
    page_number = request.GET.get('page')
    page_obj = paginator.get_page(page_number)
    return render(request, 'ticket/ticket_list.html',{"page_obj":page_obj,"user":user,"slug":slug,"search":search})
    

def ticket_details(request,ticket=None):

    myticket = Ticket.objects.get(ticket=ticket)
    return render(request, 'ticket/ticket_details.html',{"myticket":myticket})

def ticket_details_image(request,image_id):
    myticketimage=TicketImage.objects.get(id=image_id)
    return render(request, 'ticket/ticket_details_image.html',{"image":myticketimage})


def ticket_details_attachment(request,attachment_id):
    myticketattachment=TicketAttachment.objects.get(id=attachment_id)
    response = HttpResponse(
        myticketattachment.file,
        content_type= myticketattachment.mime_type)
    response['Content-Disposition'] = 'attachment; filename="'+myticketattachment.get_filename()+'"'
    return response

def tickets_ass_sub(request,user_ass=None, user_sub=None):

    if (user_ass is None):
        query_user_ass=Q()
    else:
        query_user_ass = Q(assigned_to__username=user_ass) 

    if (user_sub is None):
        query_user_sub=Q()
    else:
        query_user_sub = Q(subscribed_by__username=user_sub)


    if 'search' in request.GET:
        search = request.GET['search']
        query =  (Q(assigned_to__username__icontains=search) | Q(abstract__icontains=search) | Q(stationmetadata__slug__icontains=search))
    else:
        query = Q()
        search = None

    query = query & query_user_ass & query_user_sub
                         
    mytickets = Ticket.objects.filter(query)

    paginator = Paginator(mytickets, 25) # Show 25 contacts per page.
    page_number = request.GET.get('page')
    page_obj = paginator.get_page(page_number)
    if ( user_ass is None):
        user=user_sub
    else:
        user=user_ass
    return render(request, 'ticket/ticket_list.html',{"page_obj":page_obj,"user":user,"search":search})
    
