#ifndef PLATFORMPERMISSIONSANDROID_H
#define PLATFORMPERMISSIONSANDROID_H

#include "../platformpermissions.h"

#include <QtAndroidExtras/QtAndroid>

class PlatformPermissionsAndroid : public PlatformPermissions
{
    Q_OBJECT
public:
    explicit PlatformPermissionsAndroid(QObject *parent = nullptr);

    PermissionStatus checkPermission(Permission permission) const override;

    void requestPermission(Permission permission) override;
    void openPermissionSettings() override;

signals:

private:

    static PlatformPermissionsAndroid *s_instance;
    static void permissionResultCallback(const QtAndroid::PermissionResultMap &results);

};

#endif // PLATFORMPERMISSIONSANDROID_H
