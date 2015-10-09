#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lists.h"
#include "utility.h"

/* Add a group with name group_name to the group_list referred to by 
* group_list_ptr. The groups are ordered by the time that the group was 
* added to the list with new groups added to the end of the list.
*
* Returns 0 on success and -1 if a group with this name already exists.
*
* (I.e, allocate and initialize a Group struct, and insert it
* into the group_list. Note that the head of the group list might change
* which is why the first argument is a double pointer.) 
*/
char* add_group(Group **group_list_ptr, const char *group_name) {
	if (find_group(*group_list_ptr, group_name) == NULL) {
		//malloc space for new group
		Group *newgrp;
		newgrp = myMalloc(sizeof(Group),
			"Error allocating space for new Group");
		// set the fields of the new group node
		// first allocate space for the name
		int needed_space = strlen(group_name) + 1;	
		newgrp->name = myMalloc(needed_space,
				"Error allocating space for new Group name");
		strncpy(newgrp->name, group_name, needed_space);
		newgrp->users = NULL;
		newgrp->xcts = NULL;
		newgrp->next = NULL;
		// Need to insert into the end of the list not the front
		if (*group_list_ptr == NULL) {
			// set new head to this new group -- the list was previously empty
			*group_list_ptr = newgrp;
			return buildString("Group %s added", group_name);
		}  else {
			// walk along the list until we find the currently last group 
			Group * current = *group_list_ptr;
			while (current->next != NULL) {
				current = current->next;
			}
			// now current should be the last group 
			current->next = newgrp;			
			return buildString("Group %s added", group_name);
		}
	} else {
		return buildString("Group %s already exists", group_name);
	}
}

/* Print to standard output the names of all groups in group_list, one name
*  per line. Output is in the same order as group_list.
*/
char* list_groups(Group *group_list) {
	Group * current = group_list;
	char* outputString = NULL;
	char* tmpString = NULL;
	// create pipe for output 
	if (current != NULL){
		outputString = buildString("%s", current->name);
		current = current->next;
	}
	while (current != NULL) {
		tmpString = outputString;
		outputString = buildString("%s\n%s", outputString, current->name);
		free(tmpString);
		current = current->next;
	}
	// retunr the string
	return outputString;

}

/* Search the list of groups for a group with matching group_name
* If group_name is not found, return NULL, otherwise return a pointer to the 
* matching group list node.
*/
Group *find_group(Group *group_list, const char *group_name) {
	Group *current = group_list;
	while (current != NULL) {
		if (strcmp(current->name, group_name) == 0) return current;
		current = current->next;
	}
	return current;
}

/* Add a new user with the specified user name to the specified group. Return zero
* on success and -1 if the group already has a user with that name.
* (allocate and initialize a User data structure and insert it into the
* appropriate group list)
*/
char* add_user(Group *group, const char *user_name) {
	User *this_user = find_prev_user(group, user_name);
	if (this_user != NULL) {	
		return buildString("%s is already in group %s", user_name, group->name);
	}
	// ok to add a user to this group by this name
	// since users are stored by balance order and the new user has 0 balance
	// he goes first
	User *newuser;
	if ((newuser = malloc(sizeof(User))) == NULL) {
		perror("Error allocating space for new User");
		exit(1);
	}
	// set the fields of the new user node
	// first allocate space for the name
	int name_len = strlen(user_name);
	if ((newuser->name = malloc(name_len + 1)) == NULL) {
		perror("Error allocating space for new User name");
		exit(1);
	}
	strncpy(newuser->name, user_name, name_len + 1);
	newuser->balance = 0.0;

	// insert this user at the front of the list
	newuser->next = group->users;
	group->users = newuser;
	return buildString("Successfully added %s to the group %s", user_name, group->name);
}

/* Print to standard output the names and balances of all the users in group,
* one per line, and in the order that users are stored in the list, namely 
* lowest payer first.
*/
char* list_users(Group *group) {
	User *current_user = group->users;
	char* outputString = NULL;
	char* tmpString = NULL;
	if (current_user != NULL){
		outputString = buildString("%s %.2f", current_user->name, current_user->balance);
		current_user = current_user->next;
	}
	while (current_user != NULL) {
		tmpString = outputString;
		outputString = buildString("%s\n%s %.2f",
						outputString,
						current_user->name,
						current_user->balance);
		free(tmpString);
		current_user = current_user->next;
	}
	return outputString;
}

/* Print to standard output the balance of the specified user. Return 0
* on success, or -1 if the user with the given name is not in the group.
*/
char* user_balance(Group *group, const char *user_name) {
	User * prev_user = find_prev_user(group, user_name);
	if (prev_user == NULL) { 
		return buildString("There is no user in group %s with name %s", group->name, user_name);
	}
	if (prev_user == group->users) {
		// user could be first or second since previous is first
		if (strcmp(user_name, prev_user->name) == 0) {
			// this is the special case of first user
			return buildString("%.2f", prev_user->balance);;
		}
	}
	return buildString("%.2f", prev_user->next->balance);
}

/* Return a pointer to the user prior to the one in group with user_name. If 
* the matching user is the first in the list (i.e. there is no prior user in 
* the list), return a pointer to the matching user itself. If no matching user 
* exists, return NULL. 
*
* The reason for returning the prior user is that returning the matching user 
* itself does not allow us to change the user that occurs before the
* matching user, and some of the functions you will implement require that
* we be able to do this.
*/
User *find_prev_user(Group *group, const char *user_name) {
    User * current_user = group->users;
    // return NULL for no users in this group
    if (current_user == NULL) { 
        return NULL;
    }
    // special case where user we want is first
    if (strcmp(current_user->name, user_name) == 0) {
        return current_user;
    }
    while (current_user->next != NULL) {
        if (strcmp(current_user->next->name, user_name) == 0) {
            // we've found the user so return the previous one
            return current_user;
        }
    current_user = current_user->next;
    }
    // if we get this far without returning, current_user is last,
    // and we have already checked the last element
    return NULL;
}

/* Add the transaction represented by user_name and amount to the appropriate 
* transaction list, and update the balances of the corresponding user and group. 
* Note that updating a user's balance might require the user to be moved to a
* different position in the list to keep the list in sorted order. Returns 0 on
* success, and -1 if the specified user does not exist.
*/
char* add_xct(Group *group, const char *user_name, double amount) {
    User *this_user;
    User *prev = find_prev_user(group, user_name);
    if (prev == NULL) {
	return buildString("There is no user in group %s with name %s", group->name, user_name);
    }
    // but find_prev_user gets the PREVIOUS user, so correct
    if (prev == group->users) {
        // user could be first or second since previous is first
        if (strcmp(user_name, prev->name) == 0) {
            // this is the special case of first user
            this_user = prev;
        } else {
            this_user = prev->next;
        }
    } else {
        this_user = prev->next;
    }

    Xct *newxct;
    if ((newxct = malloc(sizeof(Xct))) == NULL) {
        perror("Error allocating space for new Xct");
        exit(1);
    }
    // set the fields of the new transaction node
    // first allocate space for the name
    int needed_space = strlen(user_name) + 1;
    if ((newxct->name = malloc(needed_space)) == NULL) {
         perror("Error allocating space for new xct name");
         exit(1);
    }
    strncpy(newxct->name, user_name, needed_space);
    newxct->amount = amount;

    // insert this xct  at the front of the list
    newxct->next = group->xcts;
    group->xcts = newxct;

    // first readjust the balance
    this_user->balance = this_user->balance + amount;

    // since we are only ever increasing this user's balance they can only
    // go further towards the end of the linked list
    //   So keep shifting if the following user has a smaller balance

    while (this_user->next != NULL &&
                  this_user->balance > this_user->next->balance ) {
        // he remains as this user but the user next gets shifted
        // to be behind him
        if (prev == this_user) {
            User *shift = this_user->next;
            this_user->next = shift->next;
            prev = shift;
            prev->next = this_user;
            group->users = prev;
        } else { // ordinary case in the middle
            User *shift = this_user->next;
            prev->next = shift;
            this_user->next = shift->next;
            shift->next = this_user;
        }
    }
    return buildString("Successfully added a new xct of user %s to the group %s", user_name, group->name);
}


