/*
 * Copyright (C) 2019 by Sukchan Lee <acetcom@gmail.com>
 *
 * This file is part of Open5GS.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "namf-handler.h"

#include "nas-path.h"
#include "gmm-build.h"

int amf_namf_comm_handle_n1_n2_message_transfer(
        ogs_sbi_session_t *session, ogs_sbi_message_t *recvmsg)
{
    int status;

    amf_ue_t *amf_ue = NULL;
    amf_sess_t *sess = NULL;

    ogs_pkbuf_t *n1smbuf = NULL;
    ogs_pkbuf_t *n2smbuf = NULL;

    char *supi = NULL;
    uint8_t pdu_session_id = OGS_NAS_PDU_SESSION_IDENTITY_UNASSIGNED;

    ogs_sbi_message_t sendmsg;
    ogs_sbi_response_t *response = NULL;

    OpenAPI_n1_n2_message_transfer_req_data_t *N1N2MessageTransferReqData;
    OpenAPI_n1_n2_message_transfer_rsp_data_t N1N2MessageTransferRspData;
    OpenAPI_n1_message_container_t *n1MessageContainer = NULL;
    OpenAPI_ref_to_binary_data_t *n1MessageContent = NULL;
    OpenAPI_n2_info_container_t *n2InfoContainer = NULL;
    OpenAPI_n2_sm_information_t *smInfo = NULL;
    OpenAPI_n2_info_content_t *n2InfoContent = NULL;
    OpenAPI_ref_to_binary_data_t *ngapData = NULL;

    ogs_assert(session);
    ogs_assert(recvmsg);

    N1N2MessageTransferReqData = recvmsg->N1N2MessageTransferReqData;
    if (!N1N2MessageTransferReqData) {
        ogs_error("No N1N2MessageTransferReqData");
        return OGS_ERROR;
    }

    n1MessageContainer = N1N2MessageTransferReqData->n1_message_container;
    if (!n1MessageContainer) {
        ogs_error("No n1MessageContainer");
        return OGS_ERROR;
    }
    n1MessageContent = n1MessageContainer->n1_message_content;
    if (!n1MessageContent || !n1MessageContent->content_id) {
        ogs_error("No n1MessageContent");
        return OGS_ERROR;
    }

    n2InfoContainer = N1N2MessageTransferReqData->n2_info_container;
    if (!n2InfoContainer) {
        ogs_error("No n2InfoContainer");
        return OGS_ERROR;
    }
    smInfo = n2InfoContainer->sm_info;
    if (!smInfo) {
        ogs_error("No smInfo");
        return OGS_ERROR;
    }
    n2InfoContent = smInfo->n2_info_content;
    if (!n2InfoContent) {
        ogs_error("No n2InfoContent");
        return OGS_ERROR;
    }
    ngapData = n2InfoContent->ngap_data;
    if (!ngapData || !ngapData->content_id) {
        ogs_error("No ngapData");
        return OGS_ERROR;
    }

    pdu_session_id = N1N2MessageTransferReqData->pdu_session_id;
    if (pdu_session_id == OGS_NAS_PDU_SESSION_IDENTITY_UNASSIGNED) {
        ogs_error("No PDU Session Identity");
        return OGS_ERROR;
    }

    supi = recvmsg->h.resource.component[1];
    if (!supi) {
        ogs_error("No SUPI");
        return OGS_ERROR;
    }

    amf_ue = amf_ue_find_by_supi(supi);
    if (!amf_ue) {
        ogs_error("No UE context [%s]", supi);
        return OGS_ERROR;
    }

    sess = amf_sess_find_by_psi(amf_ue, pdu_session_id);
    if (!sess) {
        ogs_error("[%s] No PDU Session Context [%d]",
                amf_ue->supi, pdu_session_id);
        return OGS_ERROR;
    }

    n1smbuf = ogs_sbi_find_part_by_content_id(
            recvmsg, n1MessageContent->content_id);
    if (!n1smbuf) {
        ogs_error("[%s] No N1 SM Content", amf_ue->supi);
        return OGS_ERROR;
    }
    n1smbuf = ogs_pkbuf_copy(n1smbuf);
    ogs_assert(n1smbuf);

    n2smbuf = ogs_sbi_find_part_by_content_id(recvmsg, ngapData->content_id);
    if (!n2smbuf) {
        ogs_error("[%s] No N2 SM Content", amf_ue->supi);
        return OGS_ERROR;
    }
    n2smbuf = ogs_pkbuf_copy(n2smbuf);
    ogs_assert(n2smbuf);

    nas_send_pdu_session_establishment_accept(sess, n1smbuf, n2smbuf);

    memset(&N1N2MessageTransferRspData, 0, sizeof(N1N2MessageTransferRspData));
    N1N2MessageTransferRspData.cause =
        OpenAPI_n1_n2_message_transfer_cause_N1_N2_TRANSFER_INITIATED;

    memset(&sendmsg, 0, sizeof(sendmsg));

    sendmsg.N1N2MessageTransferRspData = &N1N2MessageTransferRspData;

    status = OGS_SBI_HTTP_STATUS_OK;
    /* TODO : OGS_SBI_HTTP_STATUS_ACCEPTED */

    response = ogs_sbi_build_response(&sendmsg, status);
    ogs_assert(response);
    ogs_sbi_server_send_response(session, response);

    return OGS_OK;
}
