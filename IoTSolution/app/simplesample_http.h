// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef SIMPLESAMPLEHTTP_H
#define SIMPLESAMPLEHTTP_H

#ifdef __cplusplus
extern "C" {
#endif

    void simplesample_http_run(int pin, const char * cnnStr);
    char* simplesample_http_getUrl();
    
#ifdef __cplusplus
}
#endif

#endif /* SIMPLESAMPLEHTTP_H */