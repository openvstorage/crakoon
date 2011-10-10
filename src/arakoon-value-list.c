/*
 * This file is part of Arakoon, a distributed key-value store.
 *
 * Copyright (C) 2010 Incubaid BVBA
 *
 * Licensees holding a valid Incubaid license may use this file in
 * accordance with Incubaid's Arakoon commercial license agreement. For
 * more information on how to enter into this agreement, please contact
 * Incubaid (contact details can be found on http://www.arakoon.org/licensing).
 *
 * Alternatively, this file may be redistributed and/or modified under
 * the terms of the GNU Affero General Public License version 3, as
 * published by the Free Software Foundation. Under this license, this
 * file is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU Affero General Public License for more details.
 * You should have received a copy of the
 * GNU Affero General Public License along with this program (file "COPYING").
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <string.h>

#include "arakoon-value-list.h"
#include "arakoon-utils.h"

struct ArakoonValueListItem {
        ArakoonValueListItem *next;

        size_t value_size;
        void * value;
};

struct ArakoonValueList {
        size_t size;
        ArakoonValueListItem *first;
        ArakoonValueListItem *last;
};

struct ArakoonValueListIter {
        const ArakoonValueList *list;
        ArakoonValueListItem *current;
};

ArakoonValueList * arakoon_value_list_new(void) {
        ArakoonValueList *list = NULL;

        list = arakoon_mem_new(1, ArakoonValueList);
        RETURN_NULL_IF_NULL(list);

        list->size = 0;
        list->first = NULL;
        list->last = NULL;

        return list;
}

arakoon_rc _arakoon_value_list_prepend(ArakoonValueList *list,
    const size_t value_size, const void * const value) {
        ArakoonValueListItem *item = NULL;

        item = arakoon_mem_new(1, ArakoonValueListItem);
        RETURN_ENOMEM_IF_NULL(item);

        item->value = arakoon_mem_malloc(value_size);
        if(item->value == NULL) {
                arakoon_mem_free(item);
                return -ENOMEM;
        }

        item->next = list->first;
        item->value_size = value_size;
        memcpy(item->value, value, value_size);

        list->first = item;
        list->size = list->size + 1;

        if(list->last == NULL) {
                list->last = item;
        }

        return ARAKOON_RC_SUCCESS;
}

static arakoon_rc arakoon_value_list_append(ArakoonValueList *list,
    const size_t value_size, const void * const value) {
        ArakoonValueListItem *item = NULL;

        item = arakoon_mem_new(1, ArakoonValueListItem);
        RETURN_ENOMEM_IF_NULL(item);

        item->value = arakoon_mem_malloc(value_size);
        if(item->value == NULL) {
                arakoon_mem_free(item);
                return -ENOMEM;
        }

        item->next = NULL;
        item->value_size = value_size;
        memcpy(item->value, value, value_size);

        if(list->last != NULL) {
                list->last->next = item;
        }
        list->last = item;
        if(list->first == NULL) {
                list->first = item;
        }

        list->size = list->size + 1;

        return ARAKOON_RC_SUCCESS;
}

arakoon_rc arakoon_value_list_add(ArakoonValueList *list,
    const size_t value_size, const void * const value) {
        FUNCTION_ENTER(arakoon_value_list_add);

        return arakoon_value_list_append(list, value_size, value);
}

size_t arakoon_value_list_size(const ArakoonValueList * const list) {
        FUNCTION_ENTER(arakoon_value_list_size);

        return list->size;
}

static void arakoon_value_list_item_free(ArakoonValueListItem * const item) {
        FUNCTION_ENTER(arakoon_value_list_item_free);

        RETURN_IF_NULL(item);

        item->value_size = 0;
        arakoon_mem_free(item->value);

        arakoon_mem_free(item);
}

void arakoon_value_list_free(ArakoonValueList * const list) {
        ArakoonValueListItem *item = NULL, *olditem = NULL;

        FUNCTION_ENTER(arakoon_value_list_free);

        RETURN_IF_NULL(list);

        list->size = 0;
        item = list->first;
        list->first = NULL;

        while(item != NULL) {
                olditem = item;
                item = olditem->next;

                arakoon_value_list_item_free(olditem);
        }

        arakoon_mem_free(list);
}

ArakoonValueListIter * arakoon_value_list_create_iter(
    const ArakoonValueList * const list) {
        ArakoonValueListIter * iter = NULL;

        FUNCTION_ENTER(arakoon_value_list_create_iter);

        iter = arakoon_mem_new(1, ArakoonValueListIter);
        RETURN_NULL_IF_NULL(iter);

        iter->list = list;
        iter->current = list->first;

        return iter;
}

void arakoon_value_list_iter_free(ArakoonValueListIter * const iter) {
        FUNCTION_ENTER(arakoon_value_list_iter_free);

        RETURN_IF_NULL(iter);

        iter->list = NULL;
        iter->current = NULL;

        arakoon_mem_free(iter);
}

void arakoon_value_list_iter_next(ArakoonValueListIter * const iter,
    size_t * const value_size, const void ** const value) {
        FUNCTION_ENTER(arakoon_value_list_iter_next);

        if(iter->current != NULL) {
                *value_size = iter->current->value_size;
                *value = iter->current->value;

                iter->current = iter->current->next;
        }
        else {
                *value_size = 0;
                *value = NULL;
        }
}

void arakoon_value_list_iter_reset(ArakoonValueListIter * const iter) {
        FUNCTION_ENTER(arakoon_value_list_iter_reset);

        iter->current = iter->list->first;
}