import random
import time

people = [
	'Kate Sheehan',
	'Jeremy Sheehan',
	'Sarah Aldamigh',
	'Elliot Fabri',
	'Benjamin Eckart',
	'Luca Burak',
	'Juan Pablo Mendoza',
	'Madeline Whitehill',
	'Waleed Elsegeiny',
	'Shervin Javdani',
	'Shahrzad',
	'Josh',
	'DR',
	'Atza',
	'Woody',
	'Caitlin Rice',
	'Jimmy',
	'Ash',
]

couples = {
	'Kate Sheehan': 'Jeremy Sheehan',
	'Jeremy Sheehan': 'Kate Sheehan',
	'Sarah Aldamigh': 'Elliot Fabri',
	'Elliot Fabri': 'Sarah Aldamigh',
	'Juan Pablo Mendoza': 'Madeline Whitehill',
	'Madeline Whitehill': 'Juan Pablo Mendoza',
	'Shervin Javdani': 'Caitlin Rice',
	'Caitlin Rice': 'Shervin Javdani',
	'Shahrzad': 'Josh',
	'Josh': 'Shahrzad',
	'DR': 'Atza',
	'Atza': 'DR'
}

people_who_want_one = [
	'Kate Sheehan',
	'Jeremy Sheehan',
	'Waleed Elsegeiny',
	'Caitlin Rice',
]

num_tickets = 26

def main():
	# Concatenate all of our names together and use that as seed, since this is in order of the spreadsheet
	# which was set finalized in call and was filled in according to when people saw/had time to fill in
	# it is a constant source of pre-commitment, plus google maintains revision history so if people thing it was
	# reordered for advantage we can prove whether it was or not, I genuinely don't know what the outcome will be 
	random.seed(''.join(people)
	##time only for testing
	##random.seed(str(time.time()*1000))

	people_for_draw = set(people) - set(people_who_want_one)

	num_extra_tickets_to_assign = num_tickets - len(people)
	
	people_who_get_an_extra = random.sample(people_for_draw, num_extra_tickets_to_assign)

	people_who_need_to_be_paired = set(people) - set(people_who_get_an_extra)
	
	print '####### People who get 2#########'
	print people_who_get_an_extra
	print ''

	pairs = []

	# 1. Figure out which couples can just be paired together because only one got an extra
	for person in people_who_get_an_extra:
		if person in couples:
			if couples[person] not in people_who_get_an_extra:
				pairs.append((person, couples[person]))
				people_who_need_to_be_paired.remove(couples[person])
				people_who_get_an_extra.remove(person)

	# 2. Pair everyone who didn't get an extra

	while len(people_who_need_to_be_paired) > 1:
		person = people_who_need_to_be_paired.pop()
		if person in couples:
			if couples[person] in people_who_need_to_be_paired:
				# both partners need to be paired
				pairs.append((person, couples[person]))
				# this is safe since we are only doing it in one thread
				people_who_need_to_be_paired.remove(couples[person])

			else:
				# your partner was already paired, so pair you randomly
				# this shouldn't happen
				print "This shouldn't happen"
				partner = people_who_need_to_be_paired.pop()
				pairs.append((person, partner))

		else:
			# you aren't in a couple
			partner = people_who_need_to_be_paired.pop()
			pairs.append((person, partner))

			

	print ''
	print '####### Pairs #########'
	print pairs


if __name__ == "__main__":
    main()


