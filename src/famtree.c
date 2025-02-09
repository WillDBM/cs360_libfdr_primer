// William Armentrout
// warmentr
// Description:
//
//

// errors here are fixed when compiling
#include <stdlib.h>
#include <string.h>
#include "fields.h"
#include "jrb.h"
#include "dllist.h"

// Struct for each person
typedef struct {

	char *name;
	char sex;
	char *father;
	char *mother;
	Dllist children;

	// 0->unvisited 1->visited (no error) 2->visited (error)
	int visited;

	// 0->not printed 1->printed
	int printed;

} Person;

// large number of healper functions to make main 
// easier to read

Person* find_person(JRB tree, char* name) {
	
	if (name == NULL)
		return NULL;

	JRB node = jrb_find_str(tree, name);
	if (node == NULL)
		return NULL;
	return (Person *)node->val.v;
};

Person* insert_person(JRB tree, char* name) {
	
	Person *p = malloc(sizeof(Person));
	p->name = strdup(name);
	p->children = new_dllist();
	jrb_insert_str(tree, p->name, new_jval_v((void *)p));

	return p;
};

void free_all_persons(JRB tree) {
	
	JRB temp;
	Person *person;

	jrb_traverse(temp, tree) {
		person = (Person *)temp->val.v;
		free(person->name);

		if(person->father) free(person->father);
		if(person->mother) free(person->mother);

		free(person);
	}

	jrb_free_tree(tree);
};

void person_print_info(Person* person) {

	printf("%s\n", person->name);

	// print sex
	if (person->sex == 'M')
		printf(" Sex: Male\n");
	else if (person->sex == 'F')
		printf(" Sex: Female\n");
	else 
		printf(" Sex: Unknown\n");

	// print father
	if (person->father)
		printf(" Father: %s\n", person->father);
	else
		printf(" Father: Unknown\n");

	// print mother
	if (person->mother)
		printf(" Mother: %s\n", person->mother);
	else
		printf(" Mother: Unknown\n");

	// print children
	if (dll_empty(person->children)) {
		printf(" Children: None\n");
	} else {
		printf(" Children: \n");

		Dllist dtemp;
		dll_traverse(dtemp, person->children) {
			Person *child = (Person *)dtemp->val.v;
			printf("	%s\n", child->name);
		}
	}

	printf("\n");
		 
};

int person_is_own_descenant(Person * person){
	
	// If person is visited and no children contain 
	// the person return 0
	if (person->visited == 1) return 0;

	// If person is visited while searching it's own
	// children return 1
	if (person->visited == 2) return 1;

	person->visited = 2;

	// loop and check for cycled descendants
	Dllist dtemp;
	dll_traverse(dtemp, person->children) {
		Person *child = (Person *)dtemp->val.v;
		if (person_is_own_descenant(child)) return 1;
	}

	person->visited = 1;

	return 0;
};

int person_has_child(Person* person, char *child_name) {
	
	Dllist dtemp;
	dll_traverse(dtemp, person->children) {
		Person *child = (Person *)dtemp->val.v;
		if(strcmp(child->name, child_name) == 0) return 1;
	}

	return 0;
}

int person_set_sex(Person* person, char sex) {
	
	if (person->sex && person->sex != sex)
		return 1;

	person->sex = sex;
	return 0;
}

int person_set_parent(Person* child, Person* parent, int father) {
	
	if (parent->sex && parent->sex != (father ? 'M' : 'F')) {
		return 2;
	}

	if (father) {
		
		if(child->father == NULL) {
			child->father = strdup(parent->name);
			return 0;
		} else if(strcmp(child->father, parent->name) != 0) {
			// return 1 if child has father w/ different name
			return 1;
		} else {
			return 0;
		}
	} else {
		
		if(child->mother == NULL) {
			child->mother = strdup(parent->name);
			return 0;
		} else if (strcmp(child->mother, parent->name) != 0) {
			return 1;
		} else {
			return 0;
		}
	}

}

void person_add_child(Person* parant, Person* child) {
	
	if (!person_has_child(parant, child->name)) {
		dll_append(parant->children, new_jval_v((void *)child));
	}
}

int person_parents_printed(JRB tree, Person* person) {
	
	if (person->mother == NULL && person->father == NULL)
		return 1;

	Person *mother = find_person(tree, person->mother);
	Person *father = find_person(tree, person->father);

	if (mother && !mother->printed) 
		return 0;
	if (father && !father->printed)
		return 0;

	return 1;
}

char* get_line_value(IS is) {
	int nsize;
	char *value;

	// find size of string
	nsize = strlen(is->fields[1]);
	for (int i = 2; i < is->NF; i++) nsize += (strlen(is->fields[i]) + 1);

	// allocate memory
	value = (char *)malloc(sizeof(char) * (nsize + 1));
	strcpy(value, is->fields[1]);

	// 
	nsize = strlen(is->fields[1]);
	for (int i = 2; i < is->NF; i++) {
		value[nsize] = ' ';
		strcpy(value + nsize + 1, is->fields[i]);
		nsize += strlen(value + nsize);
	}

	return value;
}

int main() {

	JRB tree, temp;
	Dllist dtemp;
	IS is;
	Person *person;

	int nsize;

	// read input
	is = new_inputstruct(NULL);
	tree = make_jrb();

	while (get_line(is) >= 0) {
		// Skip unneccesarry lines
		if (is->NF <= 1) continue;

		// Keyword is the first word in line
		char *keyword = is->fields[0];

		char *value = get_line_value(is);

		if (strcmp(keyword, "PERSON") == 0) {
			person = find_person(tree, value);

			// create the person if they don't already exist
			if (person == NULL) person = insert_person(tree, value);
		} else if (strcmp(keyword, "FATHER_OF") == 0 || strcmp(keyword, "MOTHER_OF") == 0) {
			Person *child = find_person(tree, value);

			// 0 if father 1 if mother
			int father = strcmp(keyword, "FATHER_OF") == 0 ? 1 : 0;

			if (person_set_sex(person, father ? 'M' : 'F') == 1) {
				fprintf(stderr, "Bad input - sex mismatch on line %d\n", is->line);

				// Since we're exiting, free memory
				free_all_persons(tree);
				free(value);
				jettison_inputstruct(is);
				exit(1);
			}

			// create person if they don't already exist
			if (child == NULL) 
				child = insert_person(tree, value);

			int set_parent = person_set_parent(child, person, father);
			if(set_parent == 1) {
				fprintf(stderr, "Bad input -- child with two %s on line %d\n", father ? "fathers": "mothers", is->line);

				// free memory
				free_all_persons(tree);
				free(value);
				jettison_inputstruct(is);
				exit(1);
			} else if (set_parent == 2) {
				fprintf(stderr, "Bad input - sex mismatch on line %d\n", is->line);

				// free memory
				free_all_persons(tree);
				free(value);
				jettison_inputstruct(is);
				exit(1);
			}

			person_add_child(person, child);

		} else if (strcmp(keyword, "FATHER") == 0 || strcmp(keyword, "MOTHER") == 0) {
			// 0 is a father, 1 is a mother
			int father = strcmp(keyword, "FATHER") == 0 ? 1 : 0;

			Person *parent = find_person(tree, value);

			// create parent if they don't already exist
			if (parent == NULL) 
				parent = insert_person(tree, value);

			if (person_set_sex(parent, father ? 'M' : 'F') == 1) {
				fprintf(stderr, "Bad input - sex mismatch on line %d\n", is->line);

				//free memory
				free_all_persons(tree);
				free(value);
				jettison_inputstruct(is);
				exit(1);
			}

			int set_parent = person_set_parent(person, parent, father);
			if (set_parent == 1) {
				fprintf(stderr, "Bad input -- child with two %s on line %d\n", father ? "fathers" : "mothers", is->line);

				//free memory
				free_all_persons(tree);
				free(value);
				jettison_inputstruct(is);
				exit(1);
			} else if (set_parent == 2) {
				fprintf(stderr, "Bad input - sex mismatch on line %d\n", is->line);

				//free memory
				free_all_persons(tree);
				free(value);
				jettison_inputstruct(is);
				exit(1);
			}

			person_add_child(parent, person);
		} else if (strcmp(keyword, "SEX") == 0) {
			if (person_set_sex(person, is->fields[1][0]) == 1) {
				fprintf(stderr, "Bad input - sex mismatch on line %d\n", is->line);

				//free memory
				free_all_persons(tree);
				free(value);
				jettison_inputstruct(is);
				exit(1);
			}

		}

		// free value
		free(value);
	}

		// check for inbreading/cycle
	jrb_traverse(temp, tree) {
		
		person = (Person *)temp->val.v;
		if (person_is_own_descenant(person)) {
			fprintf(stderr, "Bad input -- cycle in specification\n");

			//free memory
			free_all_persons(tree);
			jettison_inputstruct(is);
			exit(1);
		}
	}

	Dllist print_list;
	print_list = new_dllist();

	// add all to printlist
	jrb_traverse (temp, tree) {
		person = (Person *)temp->val.v;
		dll_append(print_list, new_jval_v((void *)person));
	}

	while(!dll_empty(print_list)) {
		// get first in list
		person = (Person *)dll_first(print_list)->val.v;

		// remove it from list
		dll_delete_node(dll_first(print_list));

		if(!person->printed) {
			// only print if both parents are printed
			if(person_parents_printed(tree, person)) {
				person_print_info(person);
				person->printed = 1;

				// add dhildren to list
				dll_traverse(dtemp, person->children) {
					Person *child = (Person *)dtemp->val.v;
					dll_append(print_list, new_jval_v((void *)child));
				}
			}
		}
	}
		
	// free all memory
	free_all_persons(tree);
	jettison_inputstruct(is);
	free_dllist(print_list);

	exit(0);
}



