from django.conf.urls import url
from .views import tickets_list, ticket_details, ticket_details, ticket_details_image, ticket_details_attachment, tickets_ass_sub

urlpatterns = [
    
    url(r'^tickets/$',
        tickets_list ,name='tickets-list' ),
    
    url(r'^tickets/(?P<user>[-_\w]+)/$',
        tickets_list ,name='tickets-list'),
    
    url(r'^tickets/(?P<user>[-_\w]+)/(?P<slug>[-_\w]+)/$',
        tickets_list ,name='tickets-list'),

    url(r'^ticket/(?P<ticket>[-_\w]+)/$',
        ticket_details ,name='ticket-details'),

    url(r'^ticket_image/(?P<image_id>[-_\w]+)/$',
        ticket_details_image ,name='ticket-details-image'),

    url(r'^ticket_attachment/(?P<attachment_id>[-_\w]+)/$',
        ticket_details_attachment ,name='ticket-details-attachment'),


    url(r'^tickets_assigned/(?P<user_ass>[-_\w]+)/$',
        tickets_ass_sub ,name='tickets-assigned'),

    url(r'^tickets_subscribed/(?P<user_sub>[-_\w]+)/$',
        tickets_ass_sub ,name='tickets-subscribed'),

]
    
