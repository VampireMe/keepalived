/*
 * Soft:        Keepalived is a failover program for the LVS project
 *              <www.linuxvirtualserver.org>. It monitor & manipulate
 *              a loadbalanced server pool using multi-layer checks.
 *
 * Part:        Dynamic data structure definition.
 *
 * Version:     $Id: vrrp_data.c,v 1.1.3 2003/09/29 02:37:13 acassen Exp $
 *
 * Author:      Alexandre Cassen, <acassen@linux-vs.org>
 *
 *              This program is distributed in the hope that it will be useful,
 *              but WITHOUT ANY WARRANTY; without even the implied warranty of
 *              MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *              See the GNU General Public License for more details.
 *
 *              This program is free software; you can redistribute it and/or
 *              modify it under the terms of the GNU General Public License
 *              as published by the Free Software Foundation; either version
 *              2 of the License, or (at your option) any later version.
 *
 * Copyright (C) 2001, 2002, 2003 Alexandre Cassen, <acassen@linux-vs.org>
 */

#include "vrrp_data.h"
#include "vrrp_index.h"
#include "vrrp_sync.h"
#include "vrrp.h"
#include "memory.h"
#include "utils.h"

/* Externals vars */
extern vrrp_conf_data *vrrp_data;

/* Static addresses facility function */
void
alloc_saddress(vector strvec)
{
	if (LIST_ISEMPTY(vrrp_data->static_addresses))
		vrrp_data->static_addresses = alloc_list(free_ipaddress, dump_ipaddress);
	alloc_ipaddress(vrrp_data->static_addresses, strvec, NULL);
}

/* Static routes facility function */
void
alloc_sroute(vector strvec)
{
	if (LIST_ISEMPTY(vrrp_data->static_routes))
		vrrp_data->static_routes = alloc_list(free_iproute, dump_iproute);
	alloc_route(vrrp_data->static_routes, strvec);
}

/* VRRP facility functions */
static void
free_vgroup(void *data)
{
	vrrp_sgroup *vgroup = data;

	FREE(vgroup->gname);
	free_strvec(vgroup->iname);
	free_list(vgroup->index);
	FREE_PTR(vgroup->script_backup);
	FREE_PTR(vgroup->script_master);
	FREE_PTR(vgroup->script_fault);
	FREE_PTR(vgroup->script);
	FREE(vgroup);
}
static void
dump_vgroup(void *data)
{
	vrrp_sgroup *vgroup = data;
	int i;
	char *str;

	syslog(LOG_INFO, " VRRP Sync Group = %s, %s", vgroup->gname,
	       (vgroup->state == VRRP_STATE_MAST) ? "MASTER" : "BACKUP");
	for (i = 0; i < VECTOR_SIZE(vgroup->iname); i++) {
		str = VECTOR_SLOT(vgroup->iname, i);
		syslog(LOG_INFO, "   monitor = %s", str);
	}
	if (vgroup->script_backup)
		syslog(LOG_INFO, "   Backup state transition script = %s",
		       vgroup->script_backup);
	if (vgroup->script_master)
		syslog(LOG_INFO, "   Master state transition script = %s",
		       vgroup->script_master);
	if (vgroup->script_fault)
		syslog(LOG_INFO, "   Fault state transition script = %s",
		       vgroup->script_fault);
	if (vgroup->script)
		syslog(LOG_INFO, "   Generic state transition script = '%s'",
		       vgroup->script);
	if (vgroup->smtp_alert)
		syslog(LOG_INFO, "   Using smtp notification");
}

static void
free_vrrp(void *data)
{
	vrrp_rt *vrrp = data;

	FREE(vrrp->iname);
	FREE_PTR(vrrp->send_buffer);
	FREE_PTR(vrrp->lvs_syncd_if);
	FREE_PTR(vrrp->script_backup);
	FREE_PTR(vrrp->script_master);
	FREE_PTR(vrrp->script_fault);
	FREE_PTR(vrrp->script);
	FREE(vrrp->ipsecah_counter);
	free_list(vrrp->track_ifp);
	free_list(vrrp->vip);
	free_list(vrrp->evip);
	free_list(vrrp->vroutes);
	FREE(vrrp);
}
static void
dump_vrrp(void *data)
{
	vrrp_rt *vrrp = data;

	syslog(LOG_INFO, " VRRP Instance = %s", vrrp->iname);
	if (vrrp->init_state == VRRP_STATE_BACK)
		syslog(LOG_INFO, "   Want State = BACKUP");
	else
		syslog(LOG_INFO, "   Want State = MASTER");
	syslog(LOG_INFO, "   Runing on device = %s", IF_NAME(vrrp->ifp));
	if (vrrp->mcast_saddr)
		syslog(LOG_INFO, "   Using mcast src_ip = %s",
		       inet_ntop2(vrrp->mcast_saddr));
	if (vrrp->lvs_syncd_if)
		syslog(LOG_INFO, "   Runing LVS sync daemon on interface = %s",
		       vrrp->lvs_syncd_if);
	if (vrrp->garp_delay)
		syslog(LOG_INFO, "   Gratuitous ARP delay = %d",
		       vrrp->garp_delay/TIMER_HZ);
	syslog(LOG_INFO, "   Virtual Router ID = %d", vrrp->vrid);
	syslog(LOG_INFO, "   Priority = %d", vrrp->priority);
	syslog(LOG_INFO, "   Advert interval = %dsec",
	       vrrp->adver_int / TIMER_HZ);
	if (vrrp->preempt)
		syslog(LOG_INFO, "   Preempt Active");
	if (vrrp->auth_type) {
		syslog(LOG_INFO, "   Authentication type = %s",
		       (vrrp->auth_type ==
			VRRP_AUTH_AH) ? "IPSEC_AH" : "SIMPLE_PASSWORD");
		syslog(LOG_INFO, "   Password = %s", vrrp->auth_data);
	}
	if (!LIST_ISEMPTY(vrrp->track_ifp)) {
		syslog(LOG_INFO, "   Tracked interfaces = %d", LIST_SIZE(vrrp->track_ifp));
		dump_list(vrrp->track_ifp);
	}
	if (!LIST_ISEMPTY(vrrp->vip)) {
		syslog(LOG_INFO, "   Virtual IP = %d", LIST_SIZE(vrrp->vip));
		dump_list(vrrp->vip);
	}
	if (!LIST_ISEMPTY(vrrp->evip)) {
		syslog(LOG_INFO, "   Virtual IP Excluded = %d", LIST_SIZE(vrrp->evip));
		dump_list(vrrp->evip);
	}
	if (!LIST_ISEMPTY(vrrp->vroutes)) {
		syslog(LOG_INFO, "   Virtual Routes = %d", LIST_SIZE(vrrp->vroutes));
		dump_list(vrrp->vroutes);
	}
	if (vrrp->script_backup)
		syslog(LOG_INFO, "   Backup state transition script = %s",
		       vrrp->script_backup);
	if (vrrp->script_master)
		syslog(LOG_INFO, "   Master state transition script = %s",
		       vrrp->script_master);
	if (vrrp->script_fault)
		syslog(LOG_INFO, "   Fault state transition script = %s",
		       vrrp->script_fault);
	if (vrrp->script)
		syslog(LOG_INFO, "   Generic state transition script = '%s'",
		       vrrp->script);
	if (vrrp->smtp_alert)
		syslog(LOG_INFO, "   Using smtp notification");
}

void
alloc_vrrp_sync_group(char *gname)
{
	int size = strlen(gname);
	vrrp_sgroup *new;

	/* Allocate new VRRP group structure */
	new = (vrrp_sgroup *) MALLOC(sizeof (vrrp_sgroup));
	new->gname = (char *) MALLOC(size + 1);
	new->state = VRRP_STATE_INIT;
	memcpy(new->gname, gname, size);

	list_add(vrrp_data->vrrp_sync_group, new);
}

void
alloc_vrrp(char *iname)
{
	int size = strlen(iname);
	seq_counter *counter;
	vrrp_rt *new;

	/* Allocate new VRRP structure */
	new = (vrrp_rt *) MALLOC(sizeof (vrrp_rt));
	counter = (seq_counter *) MALLOC(sizeof (seq_counter));

	/* Build the structure */
	new->ipsecah_counter = counter;

	/* Set default values */
	new->wantstate = VRRP_STATE_BACK;
	new->init_state = VRRP_STATE_BACK;
	new->adver_int = TIMER_HZ;
	new->iname = (char *) MALLOC(size + 1);
	memcpy(new->iname, iname, size);

	list_add(vrrp_data->vrrp, new);
}

void
alloc_vrrp_track(vector strvec)
{
	vrrp_rt *vrrp = LIST_TAIL_DATA(vrrp_data->vrrp);

	if (LIST_ISEMPTY(vrrp->track_ifp))
		vrrp->track_ifp = alloc_list(NULL, dump_track);
	alloc_track(vrrp->track_ifp, strvec);
}

void
alloc_vrrp_vip(vector strvec)
{
	vrrp_rt *vrrp = LIST_TAIL_DATA(vrrp_data->vrrp);

	if (LIST_ISEMPTY(vrrp->vip))
		vrrp->vip = alloc_list(free_ipaddress, dump_ipaddress);
	alloc_ipaddress(vrrp->vip, strvec, vrrp->ifp);
}
void
alloc_vrrp_evip(vector strvec)
{
	vrrp_rt *vrrp = LIST_TAIL_DATA(vrrp_data->vrrp);

	if (LIST_ISEMPTY(vrrp->evip))
		vrrp->evip = alloc_list(free_ipaddress, dump_ipaddress);
	alloc_ipaddress(vrrp->evip, strvec, vrrp->ifp);
}

void
alloc_vrrp_vroute(vector strvec)
{
	vrrp_rt *vrrp = LIST_TAIL_DATA(vrrp_data->vrrp);

	if (LIST_ISEMPTY(vrrp->vroutes))
		vrrp->vroutes = alloc_list(free_iproute, dump_iproute);
	alloc_route(vrrp->vroutes, strvec);
}

/* data facility functions */
void
alloc_vrrp_buffer(void)
{
	vrrp_buffer = (char *) MALLOC(VRRP_PACKET_TEMP_LEN);
}

void
free_vrrp_buffer(void)
{
	FREE(vrrp_buffer);
}

vrrp_conf_data *
alloc_vrrp_data(void)
{
	vrrp_conf_data *new;

	new = (vrrp_conf_data *) MALLOC(sizeof (vrrp_conf_data));
	new->vrrp = alloc_list(free_vrrp, dump_vrrp);
	new->vrrp_index = alloc_mlist(NULL, NULL, 255);
	new->vrrp_index_fd = alloc_mlist(NULL, NULL, 1024+1);
	new->vrrp_sync_group = alloc_list(free_vgroup, dump_vgroup);

	return new;
}

void
free_vrrp_data(vrrp_conf_data * vrrp_data)
{
	free_list(vrrp_data->static_addresses);
	free_list(vrrp_data->static_routes);
	free_mlist(vrrp_data->vrrp_index, 255);
	free_mlist(vrrp_data->vrrp_index_fd, 1024+1);
	free_list(vrrp_data->vrrp);
	free_list(vrrp_data->vrrp_sync_group);
	FREE(vrrp_data);
}

void
dump_vrrp_data(vrrp_conf_data * vrrp_data)
{
	if (!LIST_ISEMPTY(vrrp_data->static_addresses)) {
		syslog(LOG_INFO, "------< Static Addresses >------");
		dump_list(vrrp_data->static_addresses);
	}
	if (!LIST_ISEMPTY(vrrp_data->static_routes)) {
		syslog(LOG_INFO, "------< Static Routes >------");
		dump_list(vrrp_data->static_routes);
	}
	if (!LIST_ISEMPTY(vrrp_data->vrrp)) {
		syslog(LOG_INFO, "------< VRRP Topology >------");
		dump_list(vrrp_data->vrrp);
	}
	if (!LIST_ISEMPTY(vrrp_data->vrrp_sync_group)) {
		syslog(LOG_INFO, "------< VRRP Sync groups >------");
		dump_list(vrrp_data->vrrp_sync_group);
	}
}