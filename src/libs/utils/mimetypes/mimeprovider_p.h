// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR LGPL-3.0

#pragma once

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "mimedatabase_p.h"
#include "mimemagicrule_p.h"

#include <QtCore/qdatetime.h>
#include <QtCore/qset.h>

namespace Utils {
namespace Internal {

class MimeMagicRuleMatcher;

class MimeProviderBase
{
public:
    MimeProviderBase(MimeDatabasePrivate *db);
    virtual ~MimeProviderBase() {}

    virtual bool isValid() = 0;
    virtual MimeType mimeTypeForName(const QString &name) = 0;
    virtual QStringList findByFileName(const QString &fileName, QString *foundSuffix) = 0;
    virtual QStringList parents(const QString &mime) = 0;
    virtual QString resolveAlias(const QString &name) = 0;
    virtual QStringList listAliases(const QString &name) = 0;
    virtual MimeType findByMagic(const QByteArray &data, int *accuracyPtr) = 0;
    virtual QList<MimeType> allMimeTypes() = 0;
    virtual void loadMimeTypePrivate(MimeTypePrivate &) {}
    virtual void loadIcon(MimeTypePrivate &) {}
    virtual void loadGenericIcon(MimeTypePrivate &) {}

    // Qt Creator additions
    virtual QMap<int, QList<MimeMagicRule> > magicRulesForMimeType(const MimeType &mimeType) = 0;
    virtual void setGlobPatternsForMimeType(const MimeType &mimeType, const QStringList &patterns) = 0;
    virtual void setMagicRulesForMimeType(const MimeType &mimeType, const QMap<int, QList<MimeMagicRule> > &rules) = 0;

    MimeDatabasePrivate *m_db;
protected:
    bool shouldCheck();
    QDateTime m_lastCheck;
};

///*
//   Parses the files 'mime.cache' and 'types' on demand
// */
//class MimeBinaryProvider : public MimeProviderBase
//{
//public:
//    MimeBinaryProvider(MimeDatabasePrivate *db);
//    virtual ~MimeBinaryProvider();

//    virtual bool isValid();
//    virtual MimeType mimeTypeForName(const QString &name);
//    virtual QStringList findByFileName(const QString &fileName, QString *foundSuffix);
//    virtual QStringList parents(const QString &mime);
//    virtual QString resolveAlias(const QString &name);
//    virtual QStringList listAliases(const QString &name);
//    virtual MimeType findByMagic(const QByteArray &data, int *accuracyPtr);
//    virtual QList<MimeType> allMimeTypes();
//    virtual void loadMimeTypePrivate(MimeTypePrivate &);
//    virtual void loadIcon(MimeTypePrivate &);
//    virtual void loadGenericIcon(MimeTypePrivate &);

//private:
//    struct CacheFile;

//    void matchGlobList(MimeGlobMatchResult &result, CacheFile *cacheFile, int offset, const QString &fileName);
//    bool matchSuffixTree(MimeGlobMatchResult &result, CacheFile *cacheFile, int numEntries, int firstOffset, const QString &fileName, int charPos, bool caseSensitiveCheck);
//    bool matchMagicRule(CacheFile *cacheFile, int numMatchlets, int firstOffset, const QByteArray &data);
//    QString iconForMime(CacheFile *cacheFile, int posListOffset, const QByteArray &inputMime);
//    void loadMimeTypeList();
//    void checkCache();

//    class CacheFileList : public QList<CacheFile *>
//    {
//    public:
//        CacheFile *findCacheFile(const QString &fileName) const;
//        bool checkCacheChanged();
//    };
//    CacheFileList m_cacheFiles;
//    QStringList m_cacheFileNames;
//    QSet<QString> m_mimetypeNames;
//    bool m_mimetypeListLoaded;
//};

/*
   Parses the raw XML files (slower)
 */
class MimeXMLProvider : public MimeProviderBase
{
public:
    MimeXMLProvider(MimeDatabasePrivate *db);

    bool isValid() override;
    MimeType mimeTypeForName(const QString &name) override;
    QStringList findByFileName(const QString &fileName, QString *foundSuffix) override;
    QStringList parents(const QString &mime) override;
    QString resolveAlias(const QString &name) override;
    QStringList listAliases(const QString &name) override;
    MimeType findByMagic(const QByteArray &data, int *accuracyPtr) override;
    QList<MimeType> allMimeTypes() override;

    bool load(const QString &fileName, QString *errorMessage);

    // Called by the mimetype xml parser
    void addMimeType(const MimeType &mt);
    void addGlobPattern(const MimeGlobPattern &glob);
    void addParent(const QString &child, const QString &parent);
    void addAlias(const QString &alias, const QString &name);
    void addMagicMatcher(const MimeMagicRuleMatcher &matcher);

    // Qt Creator additions
    void addData(const QString &id, const QByteArray &data);
    QMap<int, QList<MimeMagicRule> > magicRulesForMimeType(const MimeType &mimeType) override;
    void setGlobPatternsForMimeType(const MimeType &mimeType, const QStringList &patterns) override;
    void setMagicRulesForMimeType(const MimeType &mimeType, const QMap<int, QList<MimeMagicRule> > &rules) override;

private:
    void ensureLoaded();
    void load(const QString &fileName);

    bool m_loaded;

    typedef QHash<QString, MimeType> NameMimeTypeMap;
    NameMimeTypeMap m_nameMimeTypeMap;

    typedef QHash<QString, QString> AliasHash;
    AliasHash m_aliases;

    typedef QHash<QString, QStringList> ParentsHash;
    ParentsHash m_parents;
    MimeAllGlobPatterns m_mimeTypeGlobs;

    QList<MimeMagicRuleMatcher> m_magicMatchers;

    // Qt Creator additions
    QHash<QString, QByteArray> m_additionalData; // id -> data
};

} // Internal
} // Utils
