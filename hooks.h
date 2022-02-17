#ifndef DATA_HOOK_H
#define DATA_HOOK_H

#include <qglobal.h>

QT_BEGIN_NAMESPACE
class QObject;
QT_END_NAMESPACE

extern "C" {
extern Q_DECL_EXPORT void gammaray_startup_hook();
extern Q_DECL_EXPORT void gammaray_addObject(QObject *obj);
extern Q_DECL_EXPORT void gammaray_removeObject(QObject *obj);

/** Entry point for startup injection. */
extern Q_DECL_EXPORT void gammaray_probe_inject();

/** Entry point for runtime attaching.
 *  This differs from the above by also attempting to re-send
 *  the server address to the launcher. So only use this if you
 *  are sure there is a launcher ready to receive this information
 *  on the other side.
 */
extern Q_DECL_EXPORT void gammaray_probe_attach();

/** Entry point for static injections. */
extern Q_DECL_EXPORT void gammaray_install_hooks();

}

namespace GammaRay {
namespace Hooks {
/** Returns @c true if we have installed the hooks.
 *  This is useful to avoid loops from preloaded hooks for example.
 */
bool hooksInstalled();

/** Install hooks, either by function overwriting or using qhooks. */
void installHooks();
}
}

#endif//DATA_HOOK_H
