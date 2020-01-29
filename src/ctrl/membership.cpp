#include "colza/controller.hpp"

namespace colza {

void controller::on_member_joined(ssg_member_id_t member_id) {
    // TODO
}

void controller::on_member_left(ssg_member_id_t member_id) {
    // TODO
}

void controller::on_member_died(ssg_member_id_t member_id) {
    // TODO
}

void controller::group_membership_update(void* uargs, ssg_member_id_t member_id, ssg_member_update_type_t update_type) {
    controller* ctrl = reinterpret_cast<controller*>(uargs);
    switch(update_type) {
    case SSG_MEMBER_JOINED:
        ctrl->on_member_joined(member_id);
        break;
    case SSG_MEMBER_LEFT:
        ctrl->on_member_left(member_id);
        break;
    case SSG_MEMBER_DIED:
        ctrl->on_member_died(member_id);
        break;
    }
}

}
