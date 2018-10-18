MCU Rally Project

Alert:
	if Using Renesas Sample Code
	the sample code is for a wrong processor be carfully and read the Manual.

Usually there is no .info file generated. Then:
	- project right click: new: File: name: ".info"
	- file content:
		TOOL_CHAIN=
		VERSION=
		TC_INSTALL=
		E2STUDIO_VERSION=
		ACTIVE_CONFIGURATION=
	- project (right click): Properties: C/C++ Build: Tool Chanin Editor: Apply
		-> that will fill the gaps in the .info file

Please never commit the .cproject file.

Branch Merge (Use this if you want to add the changes of a branch to an other branch, 
				except you just want to update your branch with the changes of the branch you started from. 
				In this case use the rebase guide described below):
	1. Checkout the branch you want to merge into (Merge Target)
	2. In Git Repositories View -> Rightclick on the branch you checked out (Marked by a small tick mark)
	3. Click on "Merge..."
	4. Select the branch which should be merged into the checked out branch
	5. Select the following options:
		- Commit
		- If a Fast-Forward, create merge commit (!!!)
	6. Click "Merge"
	7. Solve conflicts if necessary
	
Branch Rebase DO NOT REBASE IF YOU HAVE ALREADY PUSHED YOUR CHANGES 
		(Use this if the branch you started from has been changed and you want these changes in your own branch as well): 
	1. Checkout the branch you want to rebase
	2. In Git Repositories View -> Rightclick on the branch you checked out (Marked by a small tick mark)
	3. Click on "Rebase..."
	4. Select the branch on which you want to rebase your checked out branch
	5. Click "Rebase"
	6. Solve conflicts if necessary
	
	
git book: https://git-scm.com/book/de/v1