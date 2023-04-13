
const posts = document.getElementById("posts")

const URL = "https://jsonplaceholder.typicode.com/posts"
// GET 
function getPosts() 
{
    fetch(URL)
    .then(res =>  res.json())
    .then(data => {
    	let output = "<h2>Posts</h2>";
     	data.forEach(post => {
        	output += `
         	<ul>
            	<li>Post ${post.id}: ${post.title}</li>
        	</ul>
         	`
     	});
     document.getElementById("output").innerHTML = output;
    })
}

posts.addEventListener("click", getPosts);


// DELETE 
const deletePosts = document.getElementById("deletePosts");

deletePosts.addEventListener("click", (e) => {
	
	e.preventDefault();

	let uriId = "https://jsonplaceholder.typicode.com/posts/3"
	
	let options = {
		method: "DELETE",
	}
	
	fetch(uriId, options)
	.then(res => res) //
	.then(data => console.log(data, "post is deleted"))
})


// POST

const newPost = document.getElementById("newPost");


newPost.addEventListener("submit", (e) => {

	e.preventDefault();

	 fetch(URL, {
		method: 'POST',
		body: new FormData(newPost),
	})
	.then(res => res.json())
	.then(data => console.log(data.id))
});
