#include "native_misc.hpp"
#include "java_util_zip.hpp"

namespace RexVM::Native::Misc {

    constexpr auto JAVA_UTIL_ZIP_FILE_NAME = "java/util/zip/ZipFile";
    constexpr auto JAVA_UTIL_JAR_JAR_FILE_NAME = "java/util/jar/JarFile";

    void registerZipMethods(NativeManager &manager) {
        manager.regNativeMethod(JAVA_UTIL_ZIP_FILE_NAME, "initIDs", "()V", false, Native::Misc::zipInitIDs);
        manager.regNativeMethod(JAVA_UTIL_ZIP_FILE_NAME, "getEntry", "(J[BZ)J", false, Native::Misc::zipGetEntry);
        manager.regNativeMethod(JAVA_UTIL_ZIP_FILE_NAME, "freeEntry", "(JJ)V", false, Native::Misc::zipFreeEntry);
        manager.regNativeMethod(JAVA_UTIL_ZIP_FILE_NAME, "getNextEntry", "(JI)J", false, Native::Misc::zipGetNextEntry);
        manager.regNativeMethod(JAVA_UTIL_ZIP_FILE_NAME, "close", "(J)V", false, Native::Misc::zipClose);
        manager.regNativeMethod(JAVA_UTIL_ZIP_FILE_NAME, "open", "(Ljava/lang/String;IJZ)J", false, Native::Misc::zipOpen);
        manager.regNativeMethod(JAVA_UTIL_ZIP_FILE_NAME, "getTotal", "(J)I", false, Native::Misc::zipGetTotal);
        manager.regNativeMethod(JAVA_UTIL_ZIP_FILE_NAME, "startsWithLOC", "(J)Z", false, Native::Misc::zipStartsWithLOC);
        manager.regNativeMethod(JAVA_UTIL_ZIP_FILE_NAME, "getManifestNum", "(J)I", false, Native::Misc::zipGetManifestNum);
        manager.regNativeMethod(JAVA_UTIL_ZIP_FILE_NAME, "read", "(JJJ[BII)I", false, Native::Misc::zipRead);

        manager.regNativeMethod(JAVA_UTIL_ZIP_FILE_NAME, "getEntryTime", "(J)J", false, Native::Misc::zipGetEntryTime);
        manager.regNativeMethod(JAVA_UTIL_ZIP_FILE_NAME, "getEntryCrc", "(J)J", false, Native::Misc::zipGetEntryCrc);
        manager.regNativeMethod(JAVA_UTIL_ZIP_FILE_NAME, "getEntryCSize", "(J)J", false, Native::Misc::zipGetEntryCSize);
        manager.regNativeMethod(JAVA_UTIL_ZIP_FILE_NAME, "getEntrySize", "(J)J", false, Native::Misc::zipGetEntrySize);
        manager.regNativeMethod(JAVA_UTIL_ZIP_FILE_NAME, "getEntryMethod", "(J)I", false, Native::Misc::zipGetEntryMethod);
        manager.regNativeMethod(JAVA_UTIL_ZIP_FILE_NAME, "getEntryFlag", "(J)I", false, Native::Misc::zipGetEntryFlag);
        manager.regNativeMethod(JAVA_UTIL_ZIP_FILE_NAME, "getCommentBytes", "(J)[B", false, Native::Misc::zipGetCommentBytes);

        manager.regNativeMethod(JAVA_UTIL_ZIP_FILE_NAME, "getEntryBytes", "(JI)[B", false, Native::Misc::zipGetEntryBytes);
        manager.regNativeMethod(JAVA_UTIL_ZIP_FILE_NAME, "getZipMessage", "(J)Ljava/lang/String;", false, Native::Misc::zipGetZipMessage);
        manager.regNativeMethod(JAVA_UTIL_ZIP_FILE_NAME, "getCentralDirectoryBytes", "(J)[B", false, Native::Misc::zipGetCentralDirectoryBytes);

        manager.regNativeMethod(JAVA_UTIL_JAR_JAR_FILE_NAME, "getMetaInfEntryNames", "()[Ljava/lang/String;", false, Native::Misc::jarGetMetaInfEntryNames);

        
    }

    void registerMiscMethods(NativeManager &manager) {
        registerZipMethods(manager);
    }
}

