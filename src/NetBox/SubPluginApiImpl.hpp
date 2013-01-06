#pragma once

/**
 * TSubpluginApiImpl class contains implementations of callback functions
 */

#include <subplugin.hpp>

namespace netbox {

class TSubpluginApiImpl
{
public:
  static void InitAPI(DCCore& dcCore);
  static void ReleaseAPI();

private:
  // nb_core_t
  static intf_handle_t NBAPI register_interface(
    subplugin_t * subplugin, const wchar_t * guid, nbptr_t funcs);
  static nb_interface_t * NBAPI query_interface(
    subplugin_t * subplugin, const wchar_t * guid, intptr_t version);
  static nbBool NBAPI release_interface(
    subplugin_t * subplugin, intf_handle_t hInterface);
};

} // namespace netbox
