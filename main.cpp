#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "Character.h"

using namespace std;

//  Gives the calculated damage to the defender if damage is bigger than 0.
void giveDamage(string *result, int damage, int defenderIndex, int *firstCommunityDeaths, int *secondCommunityDeaths, int *lastDeathOfFirst, int *lastDeathOfSecond, vector<Character>* characters) {
	if(damage>0) {
		// Controls whether the health of defender is bigger than damage. If so, reduces the health of the defender.
		if(characters->at(defenderIndex).remainingHealth > damage) {
			characters->at(defenderIndex).remainingHealth = characters->at(defenderIndex).remainingHealth - damage;
		} else {
			// If health is less than damage, kills the defender and sets its fields accordingly.
			characters->at(defenderIndex).remainingHealth = 0;
			characters->at(defenderIndex).isAlive = false;
			// Sets the lastDeath index of its community and increase the number of total deaths in that community.
			if(defenderIndex<5) {
				*firstCommunityDeaths = *firstCommunityDeaths + 1;
				*lastDeathOfFirst = defenderIndex;
				// If the type of dead character is hobbit or total number of deaths in one community is more than four, it ends the war and sets the result.
				if(characters->at(defenderIndex).type == "Hobbit" || *firstCommunityDeaths >= 4) {
					*result = "Community-2";
				}
			} else {
				*secondCommunityDeaths = *secondCommunityDeaths + 1;
				*lastDeathOfSecond = defenderIndex;
				if(characters->at(defenderIndex).type == "Hobbit" || *secondCommunityDeaths >= 4) {
					*result = "Community-1";
				}
			}
		}
	}
}

// Special characters use their skills according to their features if required rounds are passed.
void specialSkill(int index, int lastDeath, int *communityDeaths, bool *doubleDamage, vector<Character>* characters) {
	// If the type of character is elves, it gives its half health to the hobbit of its community.
	if((characters->at(index).type == "Elves") && (characters->at(index).nRoundsSinceSpecial > 9)){
		int convertedHealth = characters->at(index).remainingHealth / 2;
		characters->at(index).remainingHealth = characters->at(index).remainingHealth - convertedHealth;
		for(int i=5*(index/5); i<(5*(index/5) + 5); i++) {
			if(characters->at(i).type == "Hobbit") {
				characters->at(i).remainingHealth = characters->at(i).remainingHealth + convertedHealth;
			}
		}
		characters->at(index).nRoundsSinceSpecial = -1;
	}

	// If the type of character is dwarfs, the damage it will give doubles.
	if((characters->at(index).type == "Dwarfs") && (characters->at(index).nRoundsSinceSpecial > 19)){
		*doubleDamage = true;
		characters->at(index).nRoundsSinceSpecial = -1;
	} 

	// If the type of character is wizards, it revives the last death character of its community. If it lives, fulls its health.
	if((characters->at(index).type == "Wizards") && (characters->at(index).nRoundsSinceSpecial > 49)){
		if(lastDeath > -1) {
			// If it is revived, sets the nRoundsSinceSpecial to zero.
			if(!characters->at(lastDeath).isAlive) {
				characters->at(lastDeath).nRoundsSinceSpecial = 0;
				*communityDeaths = *communityDeaths - 1;
			}
			characters->at(lastDeath).remainingHealth = characters->at(lastDeath).healthHistory[0];
			characters->at(lastDeath).isAlive = true;
			characters->at(index).nRoundsSinceSpecial = -1;
		}
	}
}

// Change the index of attacker or defender if the character is not alive.
int changeCharacter(int index, vector<Character>* characters) {
	bool isChanged = false; // To understand whether the character whose name is bigger than the attacker or defender exists.
	if((index != 4) && (index != 9)) {
		for(int i=index+1; i<(5*(index/5) + 5); i++) {
			if(characters->at(i).isAlive) {
				index = i;
				isChanged = true;
				return index;
			}
		}
	}
	// If alphabetically bigger character does not exist, finds the first character whose name is smaller.
	if(!isChanged) {
		for(int i=index-1; i>=5*(index/5); i--) {
			if(characters->at(i).isAlive) {
				index = i;
				return index;
			}
		}
	}
}

// Sorts the vector alphabetically according the name of the characters.
void sortVector(vector<Character>* characters) {
	int smallest = 0; // To understand whether the location of should change.
	// Sorts the first community.
	for(int i=0; i<5; i++) {
		smallest = i;
		for(int j=i; j<5; j++) {
			if(characters->at(j) < characters->at(smallest)) {
				smallest = j;
			}
		}
		if(smallest != i) {
			swap(characters->at(smallest), characters->at(i));
		}
	}

	smallest = 5;
	// Sorts the second community.
	for(int i=5; i<10; i++) {
		smallest = i;
		for(int j=i; j<10; j++) {
			if(characters->at(j) < characters->at(smallest)) {
				smallest = j;
			}
		}
		if(smallest != i) {
			swap(characters->at(smallest), characters->at(i));
		}
	}
}

int main(int argc, char* argv[]) {
	ifstream infile;
	infile.open(argv[1]);

	ofstream outfile;
	outfile.open(argv[2]);

	int maxNumberOfRounds; // Maximum number of rounds that war end if unordinary situation does not occur.
	infile >> maxNumberOfRounds;
	
	// The vector that stores all character objects.
	vector<Character> characters;
	characters.reserve(10);
	// The vector stores the names according to the given order to print to the output file at the end.
	vector<string> unsortedNames;
	unsortedNames.reserve(10);

	string name, type; // Represents the name and type of the character.
	int attack, defense, health; // Represents the attack and defense point, health of the character.
	for(int i=0; i<10; i++) {
		infile >> name >> type >> attack >> defense >> health;
		characters.push_back(Character(name, type, attack, defense, health, maxNumberOfRounds));
		unsortedNames.push_back(name);
	}

	// To sort the vector alphabetically.
	sortVector(&characters);

	int passedRounds = 0; // Represent the number of rounds passed since the beginning of the war.
	string result = "Draw"; // Represents the result of the war.
	int firstCommunityDeaths = 0; // The total number of deaths in the first community.
	int secondCommunityDeaths = 0; // The total number of deaths in the second community.
	int lastDeathOfFirst = -1; // The index represents the location of last dead character of first community.
	int lastDeathOfSecond = -1; // The index represents the location of last dead character of second community.
	string attackerName, defenderName, ifSpecial; // Name of the attacker and defender, the string represents whether special skill will be used.
	int attackerIndex; // Index in the vector represents the location of attacker.
	int defenderIndex; // Index in the vector represents the location of defender.

	// War begins
	for(int i=0; i<maxNumberOfRounds; i++) {
		infile >> attackerName >> defenderName >> ifSpecial;
		// Find the index of attacker and defender according to their names.
		for(int i=0; i<10; i++) {
			if(attackerName == characters[i].name) {
				attackerIndex = i;
			}

			if(defenderName == characters[i].name) {
				defenderIndex = i;
			}
		}

		// If the attacker is not alive, changes it with the next available character according to its name.
		if(!characters[attackerIndex].isAlive) {
			attackerIndex = changeCharacter(attackerIndex, &characters);
		}

		// If the defender is not alive, changes it with the next available character according to its name.
		if(!characters[defenderIndex].isAlive) {
			defenderIndex = changeCharacter(defenderIndex, &characters);
		}

		bool doubleDamage = false; // To understand whether the damage will be doubled.

		// If it is the special round, calls the function to use special skill if it is available.
		if(ifSpecial == "SPECIAL" && attackerIndex<5) {
			specialSkill(attackerIndex, lastDeathOfFirst, &firstCommunityDeaths, &doubleDamage, &characters);
		}

		// If it is the special round, calls the function to use special skill if it is available.
		if(ifSpecial == "SPECIAL" && attackerIndex>=5) {
			specialSkill(attackerIndex, lastDeathOfSecond, &secondCommunityDeaths, &doubleDamage, &characters);
		}

		// Calculate the damage that will be given to the defender and doubles if necessary.
		int damage = characters[attackerIndex].attack - characters[defenderIndex].defense;
		if(doubleDamage) {
			damage = damage + damage;
		}

		// Calls the function to give the damage to the defender.
		giveDamage(&result, damage, defenderIndex, &firstCommunityDeaths, &secondCommunityDeaths, &lastDeathOfFirst, &lastDeathOfSecond, &characters);

		// Updates the health records and the number of total rounds passed since the special skill usage of the characters.
		for(int i=0; i<10; i++) {
			characters[i].healthHistory[passedRounds+1] = characters[i].remainingHealth;
			characters[i].nRoundsSinceSpecial++;
		}

		// Increments the number of passed rounds and ends the war if any condition ends the war is occurred.
		passedRounds++;
		if(result != "Draw") {
			break;
		}
	}

	// Changes the order of characters according to the given order at the beginning.
	for(int i=0; i<10; i++) {
		for(int j=i; j<10; j++) {
			if(characters[j].name == unsortedNames[i]) {
				swap(characters[i], characters[j]);
			}
		}
	}

	// Prints the result of the war, the number of rounds passed and total number of deaths.
	outfile << result << endl << passedRounds << endl << (firstCommunityDeaths + secondCommunityDeaths) << endl;
	// Prints the health records of the characters with their names.
	for(int i=0; i<10; i++) {
		outfile << characters[i].name << " ";
		for(int j=0; j< passedRounds + 1; j++) {
			outfile << characters[i].healthHistory[j] << " ";
		}
		outfile << endl;
	}

	infile.close();
	outfile.close();
    return 0;
}