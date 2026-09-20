#include <F4SE/F4SE.h>

F4SE_PLUGIN_VERSION = []() noexcept {
    F4SE::PluginVersionData v{};
    v.PluginVersion({ 0, 1, 0, 0 });
    v.PluginName("F4Traversal");
    v.AuthorName("local");
    v.UsesSigScanning(false);
    v.UsesAddressLibrary(true);
    v.HasNoStructUse(false);
    v.IsLayoutDependent(true);
    v.CompatibleVersions({ F4SE::RUNTIME_LATEST });
    v.MinimumRequiredXSEVersion({ 0, 0, 0, 0 });
    return v;
}();
