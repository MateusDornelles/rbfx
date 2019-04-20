//
// Copyright (c) 2008-2019 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include "../Precompiled.h"

#include "../Core/Context.h"
#include "../Core/CoreEvents.h"
#include "../Core/Profiler.h"
#include "../Core/WorkQueue.h"
#include "../IO/FileSystem.h"
#include "../IO/FileWatcher.h"
#include "../IO/Log.h"
#include "../IO/PackageFile.h"
#include "../Resource/BackgroundLoader.h"
#include "../Resource/Image.h"
#include "../Resource/JSONFile.h"
#include "../Resource/PListFile.h"
#include "../Resource/ResourceCache.h"
#include "../Resource/ResourceEvents.h"
#include "../Resource/XMLFile.h"

#include "../DebugNew.h"

#include <cstdio>

namespace Urho3D
{

static const char* checkDirs[] =
{
    "Fonts",
    "Materials",
    "Models",
    "Music",
    "Objects",
    "Particle",
    "PostProcess",
    "RenderPaths",
    "Scenes",
    "Scripts",
    "Sounds",
    "Shaders",
    "Techniques",
    "Textures",
    "UI",
    nullptr
};

static const stl::shared_ptr<Resource> noResource;

ResourceCache::ResourceCache(Context* context) :
    Object(context),
    autoReloadResources_(false),
    returnFailedResources_(false),
    searchPackagesFirst_(true),
    isRouting_(false),
    finishBackgroundResourcesMs_(5)
{
    // Register Resource library object factories
    RegisterResourceLibrary(context_);

#ifdef URHO3D_THREADING
    // Create resource background loader. Its thread will start on the first background request
    backgroundLoader_ = new BackgroundLoader(this);
#endif

    // Subscribe BeginFrame for handling directory watchers and background loaded resource finalization
    SubscribeToEvent(E_BEGINFRAME, URHO3D_HANDLER(ResourceCache, HandleBeginFrame));
}

ResourceCache::~ResourceCache()
{
#ifdef URHO3D_THREADING
    // Shut down the background loader first
    backgroundLoader_.reset();
#endif
}

bool ResourceCache::AddResourceDir(const String& pathName, unsigned priority)
{
    MutexLock lock(resourceMutex_);

    auto* fileSystem = GetSubsystem<FileSystem>();
    if (!fileSystem || !fileSystem->DirExists(pathName))
    {
        URHO3D_LOGERROR("Could not open directory " + pathName);
        return false;
    }

    // Convert path to absolute
    String fixedPath = SanitateResourceDirName(pathName);

    // Check that the same path does not already exist
    for (unsigned i = 0; i < resourceDirs_.size(); ++i)
    {
        if (!resourceDirs_[i].Compare(fixedPath, false))
            return true;
    }

    if (priority < resourceDirs_.size())
        resourceDirs_.insert(priority, fixedPath);
    else
        resourceDirs_.push_back(fixedPath);

    // If resource auto-reloading active, create a file watcher for the directory
    if (autoReloadResources_)
    {
        stl::shared_ptr<FileWatcher> watcher(new FileWatcher(context_));
        watcher->StartWatching(fixedPath, true);
        fileWatchers_.push_back(watcher);
    }

    URHO3D_LOGINFO("Added resource path " + fixedPath);
    return true;
}

bool ResourceCache::AddPackageFile(PackageFile* package, unsigned priority)
{
    MutexLock lock(resourceMutex_);

    // Do not add packages that failed to load
    if (!package || !package->GetNumFiles())
    {
        URHO3D_LOGERRORF("Could not add package file %s due to load failure", package->GetName().CString());
        return false;
    }

    if (priority < packages_.size())
        packages_.insert(priority, stl::shared_ptr<PackageFile>(package));
    else
        packages_.push_back(stl::shared_ptr<PackageFile>(package));

    URHO3D_LOGINFO("Added resource package " + package->GetName());
    return true;
}

bool ResourceCache::AddPackageFile(const String& fileName, unsigned priority)
{
    stl::shared_ptr<PackageFile> package(new PackageFile(context_));
    return package->Open(fileName) && AddPackageFile(package, priority);
}

bool ResourceCache::AddManualResource(Resource* resource)
{
    if (!resource)
    {
        URHO3D_LOGERROR("Null manual resource");
        return false;
    }

    const String& name = resource->GetName();
    if (name.Empty())
    {
        URHO3D_LOGERROR("Manual resource with empty name, can not add");
        return false;
    }

    resource->ResetUseTimer();
    resourceGroups_[resource->GetType()].resources_[resource->GetNameHash()] = resource;
    UpdateResourceGroup(resource->GetType());
    return true;
}

void ResourceCache::RemoveResourceDir(const String& pathName)
{
    MutexLock lock(resourceMutex_);

    String fixedPath = SanitateResourceDirName(pathName);

    for (unsigned i = 0; i < resourceDirs_.size(); ++i)
    {
        if (!resourceDirs_[i].Compare(fixedPath, false))
        {
            resourceDirs_.erase(i);
            // Remove the filewatcher with the matching path
            for (unsigned j = 0; j < fileWatchers_.size(); ++j)
            {
                if (!fileWatchers_[j]->GetPath().Compare(fixedPath, false))
                {
                    fileWatchers_.erase(j);
                    break;
                }
            }
            URHO3D_LOGINFO("Removed resource path " + fixedPath);
            return;
        }
    }
}

void ResourceCache::RemovePackageFile(PackageFile* package, bool releaseResources, bool forceRelease)
{
    MutexLock lock(resourceMutex_);

    for (auto i = packages_.begin(); i != packages_.end(); ++i)
    {
        if (i->get() == package)
        {
            if (releaseResources)
                ReleasePackageResources(i->get(), forceRelease);
            URHO3D_LOGINFO("Removed resource package " + (*i)->GetName());
            packages_.erase(i);
            return;
        }
    }
}

void ResourceCache::RemovePackageFile(const String& fileName, bool releaseResources, bool forceRelease)
{
    MutexLock lock(resourceMutex_);

    // Compare the name and extension only, not the path
    String fileNameNoPath = GetFileNameAndExtension(fileName);

    for (auto i = packages_.begin(); i != packages_.end(); ++i)
    {
        if (!GetFileNameAndExtension((*i)->GetName()).Compare(fileNameNoPath, false))
        {
            if (releaseResources)
                ReleasePackageResources(i->get(), forceRelease);
            URHO3D_LOGINFO("Removed resource package " + (*i)->GetName());
            packages_.erase(i);
            return;
        }
    }
}

void ResourceCache::ReleaseResource(StringHash type, const String& name, bool force)
{
    StringHash nameHash(name);
    const stl::shared_ptr<Resource>& existingRes = FindResource(type, nameHash);
    if (!existingRes)
        return;

    // If other references exist, do not release, unless forced
    if ((existingRes.use_count() == 1 && existingRes.weak_use_count() == 0) || force)
    {
        resourceGroups_[type].resources_.Erase(nameHash);
        UpdateResourceGroup(type);
    }
}

void ResourceCache::ReleaseResources(StringHash type, bool force)
{
    bool released = false;

    HashMap<StringHash, ResourceGroup>::Iterator i = resourceGroups_.Find(type);
    if (i != resourceGroups_.End())
    {
        for (HashMap<StringHash, stl::shared_ptr<Resource> >::Iterator j = i->second_.resources_.Begin();
             j != i->second_.resources_.End();)
        {
            HashMap<StringHash, stl::shared_ptr<Resource> >::Iterator current = j++;
            // If other references exist, do not release, unless forced
            if ((current->second_.use_count() == 1 && current->second_.weak_use_count() == 0) || force)
            {
                i->second_.resources_.Erase(current);
                released = true;
            }
        }
    }

    if (released)
        UpdateResourceGroup(type);
}

void ResourceCache::ReleaseResources(StringHash type, const String& partialName, bool force)
{
    bool released = false;

    HashMap<StringHash, ResourceGroup>::Iterator i = resourceGroups_.Find(type);
    if (i != resourceGroups_.End())
    {
        for (HashMap<StringHash, stl::shared_ptr<Resource> >::Iterator j = i->second_.resources_.Begin();
             j != i->second_.resources_.End();)
        {
            HashMap<StringHash, stl::shared_ptr<Resource> >::Iterator current = j++;
            if (current->second_->GetName().Contains(partialName))
            {
                // If other references exist, do not release, unless forced
                if ((current->second_.use_count() == 1 && current->second_.weak_use_count() == 0) || force)
                {
                    i->second_.resources_.Erase(current);
                    released = true;
                }
            }
        }
    }

    if (released)
        UpdateResourceGroup(type);
}

void ResourceCache::ReleaseResources(const String& partialName, bool force)
{
    // Some resources refer to others, like materials to textures. Repeat the release logic as many times as necessary to ensure
    // these get released. This is not necessary if forcing release
    bool released;
    do
    {
        released = false;

        for (HashMap<StringHash, ResourceGroup>::Iterator i = resourceGroups_.Begin(); i != resourceGroups_.End(); ++i)
        {
            for (HashMap<StringHash, stl::shared_ptr<Resource> >::Iterator j = i->second_.resources_.Begin();
                 j != i->second_.resources_.End();)
            {
                HashMap<StringHash, stl::shared_ptr<Resource> >::Iterator current = j++;
                if (current->second_->GetName().Contains(partialName))
                {
                    // If other references exist, do not release, unless forced
                    if ((current->second_.use_count() == 1 && current->second_.weak_use_count() == 0) || force)
                    {
                        i->second_.resources_.Erase(current);
                        released = true;
                    }
                }
            }
            if (released)
                UpdateResourceGroup(i->first_);
        }

    } while (released && !force);
}

void ResourceCache::ReleaseAllResources(bool force)
{
    bool released;
    do
    {
        released = false;

        for (HashMap<StringHash, ResourceGroup>::Iterator i = resourceGroups_.Begin();
             i != resourceGroups_.End(); ++i)
        {
            for (HashMap<StringHash, stl::shared_ptr<Resource> >::Iterator j = i->second_.resources_.Begin();
                 j != i->second_.resources_.End();)
            {
                HashMap<StringHash, stl::shared_ptr<Resource> >::Iterator current = j++;
                // If other references exist, do not release, unless forced
                if ((current->second_.use_count() == 1 && current->second_.weak_use_count() == 0) || force)
                {
                    i->second_.resources_.Erase(current);
                    released = true;
                }
            }
            if (released)
                UpdateResourceGroup(i->first_);
        }

    } while (released && !force);
}

bool ResourceCache::ReloadResource(Resource* resource)
{
    if (!resource)
        return false;

    resource->SendEvent(E_RELOADSTARTED);

    bool success = false;
    stl::shared_ptr<File> file = GetFile(resource->GetName());
    if (file)
        success = resource->Load(*(file.get()));

    if (success)
    {
        resource->ResetUseTimer();
        UpdateResourceGroup(resource->GetType());
        resource->SendEvent(E_RELOADFINISHED);
        return true;
    }

    // If reloading failed, do not remove the resource from cache, to allow for a new live edit to
    // attempt loading again
    resource->SendEvent(E_RELOADFAILED);
    return false;
}

void ResourceCache::ReloadResourceWithDependencies(const String& fileName)
{
    StringHash fileNameHash(fileName);
    // If the filename is a resource we keep track of, reload it
    const stl::shared_ptr<Resource>& resource = FindResource(fileNameHash);
    if (resource)
    {
        URHO3D_LOGDEBUG("Reloading changed resource " + fileName);
        ReloadResource(resource.get());
    }
    // Always perform dependency resource check for resource loaded from XML file as it could be used in inheritance
    if (!resource || GetExtension(resource->GetName()) == ".xml")
    {
        // Check if this is a dependency resource, reload dependents
        HashMap<StringHash, stl::hash_set<StringHash> >::ConstIterator j = dependentResources_.Find(fileNameHash);
        if (j != dependentResources_.End())
        {
            // Reloading a resource may modify the dependency tracking structure. Therefore collect the
            // resources we need to reload first
            stl::vector<stl::shared_ptr<Resource> > dependents;
            dependents.reserve(j->second_.size());

            for (auto k = j->second_.begin(); k != j->second_.end(); ++k)
            {
                const stl::shared_ptr<Resource>& dependent = FindResource(*k);
                if (dependent)
                    dependents.push_back(dependent);
            }

            for (unsigned k = 0; k < dependents.size(); ++k)
            {
                URHO3D_LOGDEBUG("Reloading resource " + dependents[k]->GetName() + " depending on " + fileName);
                ReloadResource(dependents[k].get());
            }
        }
    }
}

void ResourceCache::SetMemoryBudget(StringHash type, unsigned long long budget)
{
    resourceGroups_[type].memoryBudget_ = budget;
}

void ResourceCache::SetAutoReloadResources(bool enable)
{
    if (enable != autoReloadResources_)
    {
        if (enable)
        {
            for (unsigned i = 0; i < resourceDirs_.size(); ++i)
            {
                stl::shared_ptr<FileWatcher> watcher(new FileWatcher(context_));
                watcher->StartWatching(resourceDirs_[i], true);
                fileWatchers_.push_back(watcher);
            }
        }
        else
            fileWatchers_.clear();

        autoReloadResources_ = enable;
    }
}

void ResourceCache::AddResourceRouter(ResourceRouter* router, bool addAsFirst)
{
    // Check for duplicate
    for (unsigned i = 0; i < resourceRouters_.size(); ++i)
    {
        if (resourceRouters_[i] == router)
            return;
    }

    if (addAsFirst)
        resourceRouters_.push_front(stl::shared_ptr<ResourceRouter>(router));
    else
        resourceRouters_.push_back(stl::shared_ptr<ResourceRouter>(router));
}

void ResourceCache::RemoveResourceRouter(ResourceRouter* router)
{
    for (unsigned i = 0; i < resourceRouters_.size(); ++i)
    {
        if (resourceRouters_[i] == router)
        {
            resourceRouters_.erase(i);
            return;
        }
    }
}

stl::shared_ptr<File> ResourceCache::GetFile(const String& name, bool sendEventOnFailure)
{
    MutexLock lock(resourceMutex_);

    String sanitatedName = SanitateResourceName(name);
    RouteResourceName(sanitatedName, RESOURCE_GETFILE);

    if (sanitatedName.Length())
    {
        File* file = nullptr;

        if (searchPackagesFirst_)
        {
            file = SearchPackages(sanitatedName);
            if (!file)
                file = SearchResourceDirs(sanitatedName);
        }
        else
        {
            file = SearchResourceDirs(sanitatedName);
            if (!file)
                file = SearchPackages(sanitatedName);
        }

        if (file)
            return stl::shared_ptr<File>(file);
    }

    if (sendEventOnFailure)
    {
        if (resourceRouters_.size() && sanitatedName.Empty() && !name.Empty())
            URHO3D_LOGERROR("Resource request " + name + " was blocked");
        else
            URHO3D_LOGERROR("Could not find resource " + sanitatedName);

        if (Thread::IsMainThread())
        {
            using namespace ResourceNotFound;

            VariantMap& eventData = GetEventDataMap();
            eventData[P_RESOURCENAME] = sanitatedName.Length() ? sanitatedName : name;
            SendEvent(E_RESOURCENOTFOUND, eventData);
        }
    }

    return stl::shared_ptr<File>();
}

Resource* ResourceCache::GetExistingResource(StringHash type, const String& name)
{
    String sanitatedName = SanitateResourceName(name);

    if (!Thread::IsMainThread())
    {
        URHO3D_LOGERROR("Attempted to get resource " + sanitatedName + " from outside the main thread");
        return nullptr;
    }

    // If empty name, return null pointer immediately
    if (sanitatedName.Empty())
        return nullptr;

    StringHash nameHash(sanitatedName);

    const stl::shared_ptr<Resource>& existing = FindResource(type, nameHash);
    return existing;
}

Resource* ResourceCache::GetResource(StringHash type, const String& name, bool sendEventOnFailure)
{
    String sanitatedName = SanitateResourceName(name);

    if (!Thread::IsMainThread())
    {
        URHO3D_LOGERROR("Attempted to get resource " + sanitatedName + " from outside the main thread");
        return nullptr;
    }

    // If empty name, return null pointer immediately
    if (sanitatedName.Empty())
        return nullptr;

    StringHash nameHash(sanitatedName);

#ifdef URHO3D_THREADING
    // Check if the resource is being background loaded but is now needed immediately
    backgroundLoader_->WaitForResource(type, nameHash);
#endif

    const stl::shared_ptr<Resource>& existing = FindResource(type, nameHash);
    if (existing)
        return existing;

    stl::shared_ptr<Resource> resource;
    // Make sure the pointer is non-null and is a Resource subclass
    resource = DynamicCast<Resource>(context_->CreateObject(type));
    if (!resource)
    {
        URHO3D_LOGERROR("Could not load unknown resource type " + String(type));

        if (sendEventOnFailure)
        {
            using namespace UnknownResourceType;

            VariantMap& eventData = GetEventDataMap();
            eventData[P_RESOURCETYPE] = type;
            SendEvent(E_UNKNOWNRESOURCETYPE, eventData);
        }

        return nullptr;
    }

    // Attempt to load the resource
    stl::shared_ptr<File> file = GetFile(sanitatedName, sendEventOnFailure);
    if (!file)
        return nullptr;   // Error is already logged

    URHO3D_LOGDEBUG("Loading resource " + sanitatedName);
    resource->SetName(sanitatedName);

    if (!resource->Load(*(file.get())))
    {
        // Error should already been logged by corresponding resource descendant class
        if (sendEventOnFailure)
        {
            using namespace LoadFailed;

            VariantMap& eventData = GetEventDataMap();
            eventData[P_RESOURCENAME] = sanitatedName;
            SendEvent(E_LOADFAILED, eventData);
        }

        if (!returnFailedResources_)
            return nullptr;
    }

    // Store to cache
    resource->ResetUseTimer();
    resourceGroups_[type].resources_[nameHash] = resource;
    UpdateResourceGroup(type);

    return resource;
}

bool ResourceCache::BackgroundLoadResource(StringHash type, const String& name, bool sendEventOnFailure, Resource* caller)
{
#ifdef URHO3D_THREADING
    // If empty name, fail immediately
    String sanitatedName = SanitateResourceName(name);
    if (sanitatedName.Empty())
        return false;

    // First check if already exists as a loaded resource
    StringHash nameHash(sanitatedName);
    if (FindResource(type, nameHash) != noResource)
        return false;

    return backgroundLoader_->QueueResource(type, sanitatedName, sendEventOnFailure, caller);
#else
    // When threading not supported, fall back to synchronous loading
    return GetResource(type, name, sendEventOnFailure);
#endif
}

stl::shared_ptr<Resource> ResourceCache::GetTempResource(StringHash type, const String& name, bool sendEventOnFailure)
{
    String sanitatedName = SanitateResourceName(name);

    // If empty name, return null pointer immediately
    if (sanitatedName.Empty())
        return stl::shared_ptr<Resource>();

    stl::shared_ptr<Resource> resource;
    // Make sure the pointer is non-null and is a Resource subclass
    resource = DynamicCast<Resource>(context_->CreateObject(type));
    if (!resource)
    {
        URHO3D_LOGERROR("Could not load unknown resource type " + String(type));

        if (sendEventOnFailure)
        {
            using namespace UnknownResourceType;

            VariantMap& eventData = GetEventDataMap();
            eventData[P_RESOURCETYPE] = type;
            SendEvent(E_UNKNOWNRESOURCETYPE, eventData);
        }

        return stl::shared_ptr<Resource>();
    }

    // Attempt to load the resource
    stl::shared_ptr<File> file = GetFile(sanitatedName, sendEventOnFailure);
    if (!file)
        return stl::shared_ptr<Resource>();  // Error is already logged

    URHO3D_LOGDEBUG("Loading temporary resource " + sanitatedName);
    resource->SetName(file->GetName());

    if (!resource->Load(*(file.get())))
    {
        // Error should already been logged by corresponding resource descendant class
        if (sendEventOnFailure)
        {
            using namespace LoadFailed;

            VariantMap& eventData = GetEventDataMap();
            eventData[P_RESOURCENAME] = sanitatedName;
            SendEvent(E_LOADFAILED, eventData);
        }

        return stl::shared_ptr<Resource>();
    }

    return resource;
}

unsigned ResourceCache::GetNumBackgroundLoadResources() const
{
#ifdef URHO3D_THREADING
    return backgroundLoader_->GetNumQueuedResources();
#else
    return 0;
#endif
}

void ResourceCache::GetResources(stl::vector<Resource*>& result, StringHash type) const
{
    result.clear();
    HashMap<StringHash, ResourceGroup>::ConstIterator i = resourceGroups_.Find(type);
    if (i != resourceGroups_.End())
    {
        for (HashMap<StringHash, stl::shared_ptr<Resource> >::ConstIterator j = i->second_.resources_.Begin();
             j != i->second_.resources_.End(); ++j)
            result.push_back(j->second_.get());
    }
}

bool ResourceCache::Exists(const String& name) const
{
    MutexLock lock(resourceMutex_);

    String sanitatedName = SanitateResourceName(name);
    RouteResourceName(sanitatedName, RESOURCE_CHECKEXISTS);

    if (sanitatedName.Empty())
        return false;

    for (unsigned i = 0; i < packages_.size(); ++i)
    {
        if (packages_[i]->Exists(sanitatedName))
            return true;
    }

    auto* fileSystem = GetSubsystem<FileSystem>();
    for (unsigned i = 0; i < resourceDirs_.size(); ++i)
    {
        if (fileSystem->FileExists(resourceDirs_[i] + sanitatedName))
            return true;
    }

    // Fallback using absolute path
    return fileSystem->FileExists(sanitatedName);
}

unsigned long long ResourceCache::GetMemoryBudget(StringHash type) const
{
    HashMap<StringHash, ResourceGroup>::ConstIterator i = resourceGroups_.Find(type);
    return i != resourceGroups_.End() ? i->second_.memoryBudget_ : 0;
}

unsigned long long ResourceCache::GetMemoryUse(StringHash type) const
{
    HashMap<StringHash, ResourceGroup>::ConstIterator i = resourceGroups_.Find(type);
    return i != resourceGroups_.End() ? i->second_.memoryUse_ : 0;
}

unsigned long long ResourceCache::GetTotalMemoryUse() const
{
    unsigned long long total = 0;
    for (HashMap<StringHash, ResourceGroup>::ConstIterator i = resourceGroups_.Begin(); i != resourceGroups_.End(); ++i)
        total += i->second_.memoryUse_;
    return total;
}

String ResourceCache::GetResourceFileName(const String& name) const
{
    MutexLock lock(resourceMutex_);

    auto* fileSystem = GetSubsystem<FileSystem>();
    for (unsigned i = 0; i < resourceDirs_.size(); ++i)
    {
        if (fileSystem->FileExists(resourceDirs_[i] + name))
            return resourceDirs_[i] + name;
    }

    if (IsAbsolutePath(name) && fileSystem->FileExists(name))
        return name;
    else
        return String();
}

ResourceRouter* ResourceCache::GetResourceRouter(unsigned index) const
{
    return index < resourceRouters_.size() ? resourceRouters_[index] : nullptr;
}

String ResourceCache::GetPreferredResourceDir(const String& path) const
{
    String fixedPath = AddTrailingSlash(path);

    bool pathHasKnownDirs = false;
    bool parentHasKnownDirs = false;

    auto* fileSystem = GetSubsystem<FileSystem>();

    for (unsigned i = 0; checkDirs[i] != nullptr; ++i)
    {
        if (fileSystem->DirExists(fixedPath + checkDirs[i]))
        {
            pathHasKnownDirs = true;
            break;
        }
    }
    if (!pathHasKnownDirs)
    {
        String parentPath = GetParentPath(fixedPath);
        for (unsigned i = 0; checkDirs[i] != nullptr; ++i)
        {
            if (fileSystem->DirExists(parentPath + checkDirs[i]))
            {
                parentHasKnownDirs = true;
                break;
            }
        }
        // If path does not have known subdirectories, but the parent path has, use the parent instead
        if (parentHasKnownDirs)
            fixedPath = parentPath;
    }

    return fixedPath;
}

String ResourceCache::SanitateResourceName(const String& name) const
{
    // Sanitate unsupported constructs from the resource name
    String sanitatedName = GetInternalPath(name);
    sanitatedName.Replace("../", "");
    sanitatedName.Replace("./", "");

    // If the path refers to one of the resource directories, normalize the resource name
    auto* fileSystem = GetSubsystem<FileSystem>();
    if (resourceDirs_.size())
    {
        String namePath = GetPath(sanitatedName);
        String exePath = fileSystem->GetProgramDir().Replaced("/./", "/");
        for (unsigned i = 0; i < resourceDirs_.size(); ++i)
        {
            String relativeResourcePath = resourceDirs_[i];
            if (relativeResourcePath.StartsWith(exePath))
                relativeResourcePath = relativeResourcePath.Substring(exePath.Length());

            if (namePath.StartsWith(resourceDirs_[i], false))
                namePath = namePath.Substring(resourceDirs_[i].Length());
            else if (namePath.StartsWith(relativeResourcePath, false))
                namePath = namePath.Substring(relativeResourcePath.Length());
        }

        sanitatedName = namePath + GetFileNameAndExtension(sanitatedName);
    }

    return sanitatedName.Trimmed();
}

String ResourceCache::SanitateResourceDirName(const String& name) const
{
    String fixedPath = AddTrailingSlash(name);
    if (!IsAbsolutePath(fixedPath))
        fixedPath = GetSubsystem<FileSystem>()->GetCurrentDir() + fixedPath;

    // Sanitate away /./ construct
    fixedPath.Replace("/./", "/");

    return fixedPath.Trimmed();
}

void ResourceCache::StoreResourceDependency(Resource* resource, const String& dependency)
{
    if (!resource)
        return;

    MutexLock lock(resourceMutex_);

    StringHash nameHash(resource->GetName());
    stl::hash_set<StringHash>& dependents = dependentResources_[dependency];
    dependents.insert(nameHash);
}

void ResourceCache::ResetDependencies(Resource* resource)
{
    if (!resource)
        return;

    MutexLock lock(resourceMutex_);

    StringHash nameHash(resource->GetName());

    for (HashMap<StringHash, stl::hash_set<StringHash> >::Iterator i = dependentResources_.Begin(); i != dependentResources_.End();)
    {
        stl::hash_set<StringHash>& dependents = i->second_;
        dependents.erase(nameHash);
        if (dependents.empty())
            i = dependentResources_.Erase(i);
        else
            ++i;
    }
}

String ResourceCache::PrintMemoryUsage() const
{
    String output = "Resource Type                 Cnt       Avg       Max    Budget     Total\n\n";
    char outputLine[256];

    unsigned totalResourceCt = 0;
    unsigned long long totalLargest = 0;
    unsigned long long totalAverage = 0;
    unsigned long long totalUse = GetTotalMemoryUse();

    for (HashMap<StringHash, ResourceGroup>::ConstIterator cit = resourceGroups_.Begin(); cit != resourceGroups_.End(); ++cit)
    {
        const unsigned resourceCt = cit->second_.resources_.Size();
        unsigned long long average = 0;
        if (resourceCt > 0)
            average = cit->second_.memoryUse_ / resourceCt;
        else
            average = 0;
        unsigned long long largest = 0;
        for (HashMap<StringHash, stl::shared_ptr<Resource> >::ConstIterator resIt = cit->second_.resources_.Begin(); resIt != cit->second_.resources_.End(); ++resIt)
        {
            if (resIt->second_->GetMemoryUse() > largest)
                largest = resIt->second_->GetMemoryUse();
            if (largest > totalLargest)
                totalLargest = largest;
        }

        totalResourceCt += resourceCt;

        const String countString(cit->second_.resources_.Size());
        const String memUseString = GetFileSizeString(average);
        const String memMaxString = GetFileSizeString(largest);
        const String memBudgetString = GetFileSizeString(cit->second_.memoryBudget_);
        const String memTotalString = GetFileSizeString(cit->second_.memoryUse_);
        const String resTypeName = context_->GetTypeName(cit->first_);

        memset(outputLine, ' ', 256);
        outputLine[255] = 0;
        sprintf(outputLine, "%-28s %4s %9s %9s %9s %9s\n", resTypeName.CString(), countString.CString(), memUseString.CString(), memMaxString.CString(), memBudgetString.CString(), memTotalString.CString());

        output += ((const char*)outputLine);
    }

    if (totalResourceCt > 0)
        totalAverage = totalUse / totalResourceCt;

    const String countString(totalResourceCt);
    const String memUseString = GetFileSizeString(totalAverage);
    const String memMaxString = GetFileSizeString(totalLargest);
    const String memTotalString = GetFileSizeString(totalUse);

    memset(outputLine, ' ', 256);
    outputLine[255] = 0;
    sprintf(outputLine, "%-28s %4s %9s %9s %9s %9s\n", "All", countString.CString(), memUseString.CString(), memMaxString.CString(), "-", memTotalString.CString());
    output += ((const char*)outputLine);

    return output;
}

const stl::shared_ptr<Resource>& ResourceCache::FindResource(StringHash type, StringHash nameHash)
{
    MutexLock lock(resourceMutex_);

    HashMap<StringHash, ResourceGroup>::Iterator i = resourceGroups_.Find(type);
    if (i == resourceGroups_.End())
        return noResource;
    HashMap<StringHash, stl::shared_ptr<Resource> >::Iterator j = i->second_.resources_.Find(nameHash);
    if (j == i->second_.resources_.End())
        return noResource;

    return j->second_;
}

const stl::shared_ptr<Resource>& ResourceCache::FindResource(StringHash nameHash)
{
    MutexLock lock(resourceMutex_);

    for (HashMap<StringHash, ResourceGroup>::Iterator i = resourceGroups_.Begin(); i != resourceGroups_.End(); ++i)
    {
        HashMap<StringHash, stl::shared_ptr<Resource> >::Iterator j = i->second_.resources_.Find(nameHash);
        if (j != i->second_.resources_.End())
            return j->second_;
    }

    return noResource;
}

void ResourceCache::ReleasePackageResources(PackageFile* package, bool force)
{
    stl::hash_set<StringHash> affectedGroups;

    const HashMap<String, PackageEntry>& entries = package->GetEntries();
    for (HashMap<String, PackageEntry>::ConstIterator i = entries.Begin(); i != entries.End(); ++i)
    {
        StringHash nameHash(i->first_);

        // We do not know the actual resource type, so search all type containers
        for (HashMap<StringHash, ResourceGroup>::Iterator j = resourceGroups_.Begin(); j != resourceGroups_.End(); ++j)
        {
            HashMap<StringHash, stl::shared_ptr<Resource> >::Iterator k = j->second_.resources_.Find(nameHash);
            if (k != j->second_.resources_.End())
            {
                // If other references exist, do not release, unless forced
                if ((k->second_.use_count() == 1 && k->second_.weak_use_count() == 0) || force)
                {
                    j->second_.resources_.Erase(k);
                    affectedGroups.insert(j->first_);
                }
                break;
            }
        }
    }

    for (auto i = affectedGroups.begin(); i != affectedGroups.end(); ++i)
        UpdateResourceGroup(*i);
}

void ResourceCache::UpdateResourceGroup(StringHash type)
{
    HashMap<StringHash, ResourceGroup>::Iterator i = resourceGroups_.Find(type);
    if (i == resourceGroups_.End())
        return;

    for (;;)
    {
        unsigned totalSize = 0;
        unsigned oldestTimer = 0;
        HashMap<StringHash, stl::shared_ptr<Resource> >::Iterator oldestResource = i->second_.resources_.End();

        for (HashMap<StringHash, stl::shared_ptr<Resource> >::Iterator j = i->second_.resources_.Begin();
             j != i->second_.resources_.End(); ++j)
        {
            totalSize += j->second_->GetMemoryUse();
            unsigned useTimer = j->second_->GetUseTimer();
            if (useTimer > oldestTimer)
            {
                oldestTimer = useTimer;
                oldestResource = j;
            }
        }

        i->second_.memoryUse_ = totalSize;

        // If memory budget defined and is exceeded, remove the oldest resource and loop again
        // (resources in use always return a zero timer and can not be removed)
        if (i->second_.memoryBudget_ && i->second_.memoryUse_ > i->second_.memoryBudget_ &&
            oldestResource != i->second_.resources_.End())
        {
            URHO3D_LOGDEBUG("Resource group " + oldestResource->second_->GetTypeName() + " over memory budget, releasing resource " +
                     oldestResource->second_->GetName());
            i->second_.resources_.Erase(oldestResource);
        }
        else
            break;
    }
}

void ResourceCache::HandleBeginFrame(StringHash eventType, VariantMap& eventData)
{
    for (unsigned i = 0; i < fileWatchers_.size(); ++i)
    {
        FileChange change;
        while (fileWatchers_[i]->GetNextChange(change))
        {
            auto it = ignoreResourceAutoReload_.find(change.fileName_);
            if (it != ignoreResourceAutoReload_.end())
            {
                ignoreResourceAutoReload_.erase(it);
                continue;
            }

            ReloadResourceWithDependencies(change.fileName_);

            // Finally send a general file changed event even if the file was not a tracked resource
            using namespace FileChanged;

            VariantMap& eventData = GetEventDataMap();
            eventData[P_FILENAME] = fileWatchers_[i]->GetPath() + change.fileName_;
            eventData[P_RESOURCENAME] = change.fileName_;
            SendEvent(E_FILECHANGED, eventData);
        }
    }

    // Check for background loaded resources that can be finished
#ifdef URHO3D_THREADING
    {
        URHO3D_PROFILE("FinishBackgroundResources");
        backgroundLoader_->FinishResources(finishBackgroundResourcesMs_);
    }
#endif
}

File* ResourceCache::SearchResourceDirs(const String& name)
{
    auto* fileSystem = GetSubsystem<FileSystem>();
    for (unsigned i = 0; i < resourceDirs_.size(); ++i)
    {
        if (fileSystem->FileExists(resourceDirs_[i] + name))
        {
            // Construct the file first with full path, then rename it to not contain the resource path,
            // so that the file's sanitatedName can be used in further GetFile() calls (for example over the network)
            File* file(new File(context_, resourceDirs_[i] + name));
            file->SetName(name);
            return file;
        }
    }

    // Fallback using absolute path
    if (fileSystem->FileExists(name))
        return new File(context_, name);

    return nullptr;
}

File* ResourceCache::SearchPackages(const String& name)
{
    for (unsigned i = 0; i < packages_.size(); ++i)
    {
        if (packages_[i]->Exists(name))
            return new File(context_, packages_[i], name);
    }

    return nullptr;
}

void RegisterResourceLibrary(Context* context)
{
    Image::RegisterObject(context);
    JSONFile::RegisterObject(context);
    PListFile::RegisterObject(context);
    XMLFile::RegisterObject(context);
}

void ResourceCache::Scan(stl::vector<String>& result, const String& pathName, const String& filter, unsigned flags, bool recursive) const
{
    stl::vector<String> interimResult;

    for (unsigned i = 0; i < packages_.size(); ++i)
    {
        packages_[i]->Scan(interimResult, pathName, filter, recursive);
        result.insert(result.end(), interimResult.begin(), interimResult.end());
    }

    FileSystem* fileSystem = GetSubsystem<FileSystem>();
    for (unsigned i = 0; i < resourceDirs_.size(); ++i)
    {
        fileSystem->ScanDir(interimResult, resourceDirs_[i] + pathName, filter, flags, recursive);
        result.insert(result.end(), interimResult.begin(), interimResult.end());
    }
}

String ResourceCache::PrintResources(const String& typeName) const
{

    StringHash typeNameHash(typeName);

    String output = "Resource Type         Refs   WeakRefs  Name\n\n";

    for (HashMap<StringHash, ResourceGroup>::ConstIterator cit = resourceGroups_.Begin(); cit != resourceGroups_.End(); ++cit)
    {
        for (HashMap<StringHash, stl::shared_ptr<Resource> >::ConstIterator resIt = cit->second_.resources_.Begin(); resIt != cit->second_.resources_.End(); ++resIt)
        {
            Resource* resource = resIt->second_;

            // filter
            if (typeName.Length() && resource->GetType() != typeNameHash)
                continue;

            output.AppendWithFormat("%s     %i     %i     %s\n",resource->GetTypeName().CString(), resource->Refs(), resource->WeakRefs(), resource->GetName().CString());
        }

    }

    return output;
}

bool ResourceCache::RenameResource(String source, String destination)
{
    if (!packages_.empty())
    {
        URHO3D_LOGERROR("Renaming resources not supported while packages are in use.");
        return false;
    }

    if (!IsAbsolutePath(source) || !IsAbsolutePath(destination))
    {
        URHO3D_LOGERROR("Renaming resources requires absolute paths.");
        return false;
    }

    auto* fileSystem = GetSubsystem<FileSystem>();

    if (!fileSystem->FileExists(source) && !fileSystem->DirExists(source))
    {
        URHO3D_LOGERROR("Source path does not exist.");
        return false;
    }

    if (fileSystem->FileExists(destination) || fileSystem->DirExists(destination))
    {
        URHO3D_LOGERROR("Destination path already exists.");
        return false;
    }

    using namespace ResourceRenamed;

    // Ensure parent path exists
    if (!fileSystem->CreateDirsRecursive(GetPath(destination)))
        return false;

    if (!fileSystem->Rename(source, destination))
    {
        URHO3D_LOGERRORF("Renaming '%s' to '%s' failed.", source.CString(), destination.CString());
        return false;
    }

    String resourceName;
    String destinationName;
    for (const auto& dir : resourceDirs_)
    {
        if (source.StartsWith(dir))
            resourceName = source.Substring(dir.Length());
        if (destination.StartsWith(dir))
            destinationName = destination.Substring(dir.Length());
    }

    if (resourceName.Empty())
    {
        URHO3D_LOGERRORF("'%s' does not exist in resource path.", source.CString());
        return false;
    }

    // Update loaded resource information
    for (auto& groupPair : resourceGroups_)
    {
        bool movedAny = false;
        auto resourcesCopy = groupPair.second_.resources_;
        for (auto& resourcePair : resourcesCopy)
        {
            stl::shared_ptr<Resource> resource = resourcePair.second_;
            if (resource->GetName().StartsWith(resourceName))
            {
                if (autoReloadResources_)
                {
                    ignoreResourceAutoReload_.emplace_back(destinationName);
                    ignoreResourceAutoReload_.emplace_back(resourceName);
                }

                groupPair.second_.resources_.Erase(resource->GetNameHash());
                resource->SetName(destinationName);
                groupPair.second_.resources_[resource->GetNameHash()] = resource;
                movedAny = true;

                using namespace ResourceRenamed;
                SendEvent(E_RESOURCERENAMED, P_FROM, resourceName, P_TO, destinationName);
            }
        }
        if (movedAny)
            UpdateResourceGroup(groupPair.first_);
    }

    return true;
}

void ResourceCache::IgnoreResourceReload(const String& name)
{
    ignoreResourceAutoReload_.emplace_back(name);
}

void ResourceCache::IgnoreResourceReload(const Resource* resource)
{
    IgnoreResourceReload(resource->GetName());
}

void ResourceCache::RouteResourceName(String& name, ResourceRequest requestType) const
{
    name = SanitateResourceName(name);
    if (!isRouting_)
    {
        isRouting_ = true;
        for (unsigned i = 0; i < resourceRouters_.size(); ++i)
            resourceRouters_[i]->Route(name, requestType);
        isRouting_ = false;
    }
}

}
